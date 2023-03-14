package com.example.myapplication

import android.content.Intent
import android.net.Uri
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import com.microsoft.signalr.HubConnectionBuilder
import io.reactivex.rxjava3.core.Single
import okhttp3.*
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import java.io.IOException
import java.util.*

class MainActivity : AppCompatActivity() {

    private val clientId = "your-client-id"
    private val authRedirectUri = "enabledplay-samples://"
    private val deviceTypeId = "3ae3d1ed-97b7-4572-a57a-00d4724270a0" // Change this to your device type id
    private val state = UUID.randomUUID().toString()
    private var accessToken: String? = null
    private var refreshToken: String? = null
    private var authButton: Button? = null
    private var textView: TextView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        textView = findViewById(R.id.textView)
        authButton = findViewById<Button>(R.id.signInButton)
        authButton!!.setOnClickListener {
            val authUri = Uri.parse("https://services.enabledplay.com/signin")
                .buildUpon()
                .appendQueryParameter("client_id", clientId)
                .appendQueryParameter("state", state)
                .appendQueryParameter("redirect_uri", authRedirectUri)
                .build()

            val intent = Intent(Intent.ACTION_VIEW, authUri)
            startActivity(intent)
        }
    }

    override fun onResume() {
        super.onResume()

        val intent = intent
        val data: Uri? = intent?.data
        if (data != null && data.toString().startsWith(authRedirectUri)) {
            val code = data.getQueryParameter("code")
            if (code != null) {
                exchangeCodeForTokens(code)
            } else {
                // Handle the case where the user denied the authorization request
            }
        }
    }

    private val client = OkHttpClient()
    private fun exchangeCodeForTokens(code: String) {

        val body = FormBody.Builder()
            .add("client_id", clientId)
            .add("code", code)
            .build()

        val request = Request.Builder()
            .url("https://services.enabledplay.com/signin/token")
            .post(body)
            .build()

        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                // Handle the failure case
            }

            override fun onResponse(call: Call, response: Response) {
                val body = response.body?.string()
                if (response.isSuccessful && body != null) {
                    val json = JSONObject(body)
                    accessToken = json.getString("access_token")
                    refreshToken = json.getString("refresh_token")

                    saveValue("accessToken", accessToken!!)
                    saveValue("refreshToken", refreshToken!!)

                    // Create a user device
                    val userDevice = createUserDevice(deviceTypeId)
                    val userDeviceId = userDevice.getString("id")
                    val deviceToken = createUserDeviceToken(userDeviceId)
                    saveValue("userDeviceId", userDeviceId)
                    saveValue("deviceToken", deviceToken)

                    // connect to signalr hub
                    val hubConnection = HubConnectionBuilder.create("https://services.enabledplay.com/device")
                        .withHeader("X-Device-Token", deviceToken)
                        .withAccessTokenProvider(Single.defer { Single.just(accessToken!!) })
                        .build()

                    hubConnection.on("DeviceCommand", { command ->
                        runOnUiThread {
                            textView!!.text = command.toString()
                        }
                    }, String::class.java)

                    hubConnection.start().andThen { _ ->
                        hubConnection.send("VerifySelf")
                    }

                } else {
                    // Handle the non-successful response case
                }
            }
        })
    }

    fun saveValue(key: String, value: String) {
        val sharedPref = this.getPreferences(MODE_PRIVATE) ?: return
        with (sharedPref.edit()) {
            putString(key, value)
            apply()
        }
    }

    fun createUserDevice(deviceTypeId: String): JSONObject {

        val request = Request.Builder()
            .header("Authorization", "Bearer $accessToken")
            .url("https://services.enabledplay.com/api/Device/userDevices/new/$deviceTypeId")
            .post("".toRequestBody())
            .build()

        val response = client.newCall(request).execute()
        return JSONObject(response.body?.string())
    }

    fun createUserDeviceToken(userDeviceId: String): String {
        val request = Request.Builder()
            .header("Authorization", "Bearer $accessToken")
            .url("https://services.enabledplay.com/api/Device/userDevices/$userDeviceId/tokens")
            .post("".toRequestBody())
            .build()

        val response = client.newCall(request).execute()
        val responseBody = response.body?.string()
        return JSONObject(responseBody).getString("token")
    }
}
