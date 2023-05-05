Anwendung IO_API
- analogen Sensor auslesen:
	- in main:
		struct variable "IO_analogSensor_t" entsprechend IO_API.h initialisieren (member sind kommentiert)
		mit IO_readAnalogValue(IO_analogSensor_t *sensor_ptr) kann sensor objekt als pointer übergeben werden:
			- dadurch wird der aktuelle adc wert sowie der konvertierte Messwert in adcValue und currentValue gespeichert
		anschließend kann die Messung zb über void IO_printAnalogValue(IO_analogSensor_t sensor) in der console ausgegeben werden
- buttonHandler ist bereits in der while loop der main aufgerufen, jedoch wurden zum ersten Test die darin aufgerufenen funktionen durch ein print der function ersetzt
	(siehe button_API)
- motor_API enthält zb function:
	- motor_set_function(Motor_t *motor_ptr, motor_function_t function)
		setzt die digitalen Eingänge des Motors je nach "motor_function_t function", entsprechend des Datenblatts
	- 
		