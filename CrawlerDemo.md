# Introduction #

Crawler is a utility designed for testing and demonstration of the WebEngine open source library features. This program gathers information about the resources of a specified web server by analyzing references in the HTML markup, text, and JavaScript code. Additionally, a query is sent to the Web Of Trust knowledge base to obtain information about the analyzed site. This check demonstrates analysis of web application vulnerabilities.
First and foremost, please do not be evil. Use crawler only against services you own, or have a permission to test. The given application is not a full-fledged analyzer of web application security.

Furthermore, the library is currently not meant for scanning of rogue and misbehaving HTTP servers; in these cases, correct and stable operation cannot be guaranteed.
The main features provided by the application are listed below:
  * JavaScript analysis aimed at receiving references with simulation of a DOM structure
  * Support of the Basic, Digest, and NTLM authorization schemes
  * Access to the contents of web servers via HTTP
  * Operation via proxy servers with various authorization schemes
  * A wide variety of options to describe the scan target (lists of scanned domains, restriction of scanning to a host, a domain, or a web server directory, etc.)
  * Modular structure, which allows one to implement plug-ins