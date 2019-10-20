1. install unit test program.
	1) copy pltftest to your board.
	2) copy aliplatform_test_config.ini to the same directory.

2. start unit test
	1) run ./pltftest
	2) play
		- run "play dvbs1 nim1" to start play stream 1 by use NIM1.
		- run "play dvbc1 nim2" to start play stream 1 by use NIM2.
		- you can add stream or modify the NIM configration in aliplatform_test_config.ini.
	3) stop
		- run "stop" to stop the current playback.
	4) log
		- run "log" to go into log module unit test.
	5) dis
		- run "dis" to go into display module unit test.
	......

3. Please add or modify your configuration in aliplatform_test_config.ini.
	- The default configuration is only for M3515-01V03 demo board.
