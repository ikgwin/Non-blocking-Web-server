# Non-blocking-Web-server Implementation

This project showcases a non-blocking web server implemented in C, designed to handle multiple simultaneous service requests. It features:

Implementation of HTTP/1.1 protocol specifications.
Use of socket programming to manage incoming client requests.
Parsing HTTP requests and generating appropriate responses based on RFC 1945 standards.
Utilization of fork() for parallel processing of client requests.
Integration of helper methods for HTTP request parsing and response generation.
The server is capable of:

Serving static web pages.
Handling various HTTP methods (GET, HEAD) with appropriate status codes.
Error handling for invalid requests and file not found scenarios.
