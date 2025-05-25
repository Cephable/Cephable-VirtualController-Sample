const http = require("http");
const url = require("url");
const fs = require("fs");

const server = http.createServer((req, res) => {
  const parsedUrl = url.parse(req.url);
  if (parsedUrl.pathname === "/") {
    res.writeHead(200, { "Content-Type": "text/html" });
    fs.createReadStream(__dirname + "/index.html").pipe(res);
  } else {
    res.writeHead(404, { "Content-Type": "text/plain" });
    res.end("404 Not Found");
  }
});

server.listen(3000, () => {
  console.log("Server running on http://localhost:3000");
});
