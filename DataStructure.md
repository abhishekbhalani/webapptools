# Introduction #

Add your content here.



# Details #

Add your content here.  Format your content with:
  * Text in **bold** or _italic_
  * Headings, paragraphs, and lists
  * Automatic links to other wiki pages

## Scan results structure ##
```
\scan
  +-- id = '123456'
  +-- object = '987654'
  +-- profile = '191702'
  +-- start_time = '2000-Jan-01 00:01:45'
  +-- finish_time = '2000-Jan-01 00:02:04'
  +-- ping_time = '2000-Jan-01 00:02:00'
  +-- status = '0'
  +-- scan_data_list
    +-- scan_data
    | +-- id = '19221'
    | +-- scan_id= '123456'
    | +-- parent= '0'
    | +-- req_url= 'http://google.com/'
    | +-- final_url= 'http://www.google.com/?l=en'
    | +-- response = '200'
    | +-- data_size= '3040'
    | +-- download_time= '1'
    | +-- vulner_list
    |   +-- vulner
    |   | +-- id = 'pluginX:0101'
    |   | +-- title = 'Sample vulner #1'
    |   | +-- desc = 'Just a sample information treated as "vulner" of sume severity'
    |   | ... other "vulner's data" (may be variable list)
    |   +-- vulner
    |   | +-- id = 'pluginX:0202'
    |   | +-- title = 'Sample vulner #2'
    |   ... other vulners
    +-- scan_data
    | +-- id = '19222'
    | +-- scan_id= '123456'
    | +-- parent= '19221'
    ... other scan_datas
```