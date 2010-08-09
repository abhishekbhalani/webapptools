rem %echo off
set XUL_RUNNER="c:\Program Files\Mozilla Firefox\firefox.exe" -app
rem set XUL_RUNNER="c:\Tools\xulrunner\xulrunner.exe"
start "CrawlerUI" %XUL_RUNNER% application.ini
