Demo application to collect HTTP links from given web-application

At this moment only Win32 version is used in the development process.

NOTES:
boost - directory contains some boost extensions.
		Need to upgrade used library to version 1.42 and remove this dirty-hack

Google V8 javascript engine need to buil project. v8.lib must be placed in the
webEngine\libs\(Configuration) directory

USAGE:

	crawler.exe --target http://www.example.com:8080/start/url/path --result data.txt

Command-line configuration:
  --help                         this help message
  --target arg                   target to scan
  --output arg (=0)              output format: 0 - full, 1 - only links
  --depth arg (=10)              scan depth
  --dir                          stay in the directory
  --host                         stay on the host
  --domain                       stay on the domain
  --dlist                        stay in the domain's list
  --ip                           stay on the IP address
  --url_param                    ignore URL parameters
  --parallel arg (=10)           number of parallel requests
  --no-jscript                   not analyze JavaScript links
  --content arg (=0)             conent_type processing mode
  --config arg (=.\scanner.conf) configuration file
  --result arg                   results file (stdout, if not set)
  --trace arg                    trace configuration file
  --generate arg                 make config file with current values


"content" option's values:
	0 - any content-type
	1 - empty and "text/*"
	2 - only "text/*"
	3 - empty and "text/html"
	4 - only "text/html"

Config file options:
	log_file            file to store log information
	log_level           level of the log information [0=FATAL, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=TRACE]
	log_layout          layout of the log messages (see the log4cxx documentation)
	plugin_dir          directory, where plug-ins are placed (default  value is .\)
	db2_interface       plug-in identifier to connect to Storage DB (default value is mem_storage)
	db2_parameters      plug-in configuration to connect to Storage DB
	domain_list         semicolon separated list of the allowed sub-domains
	ext_deny            semicolon separated list of the denied file types (by extensions)
	depth               scan depth (default  value is 10)
	dir                 stay in the directory (0/1)
	host                stay on the host (0/1)
	domain              stay on the domain (0/1)
	dlist               stay in the domain's list (0/1)
	ip                  stay on the IP address (0/1)
	url_param           ignore URL parameters (0/1)
	parallel            number of parallel requests (default  value is 10)
	jscript             analyze JavaScript links (0/1)
	js_preload          file with JavaScripts preloads (debug routines)
	content             conent_type processing mode (default  value is 0)
	output              output format: 0 - full, 1 - only links