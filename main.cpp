
#include "AmbientData.h"
#include "MQTTApp.h"
#include "MySQL.h"




int main(){
	//MQTTApp* mqttSubscriberData = new MQTTApp(1, "mbed-sample/#", "holahola"); //subscribe values
	//MQTTApp* mqttpublisher = new MQTTApp(0, "mbed-sample/#", "holahola"); 
		
	MySQL database;
	database.queryValues();

}