#include "MQTTApp.h"
#include <iostream>
#include "signal.h"


int toStop = 0;
void handleSignal(int sig)
{
	toStop = 1;
}


MQTTApp::MQTTApp(int opt, String^ top, String^ mess)
{
	int rc = 0;
	char url[256];
	char clientId[24];
	char message[20] = { 0 };
	char topic[17];


	sprintf(topic, "%s", top);
	// Default settings:
	char* action = "publish";
	//if (top->Length < sizeof(topic)) // make sure it fits & allow space for null terminator
		//sprintf(topic, "%s", top);

	if (mess->Length < sizeof(topic)) // make sure it fits & allow space for null terminator
		sprintf(message, "%s", mess);

	int qos = 1;
	char* broker = "85.119.83.194";
	char* port = "1883";
	int i = 0;


	typedef void(*SignalHandlerPointer)(int);

	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	
	// Validate the provided arguments
	if (opt > 1)
	{
		std::cout <<"Invalid action:";
	}

	if (topic == NULL || (topic != NULL && strlen(topic) == 0))
	{
		// Set the default topic according to the specified action
		//if (strcmp(action, "publish") == 0)
			//topic = "MQTTV3Sample/C/v3";
		//else
			//topic = "MQTTV3Sample/#";
	}

	// Construct the full broker URL and clientId
	sprintf(url, "tcp://%s:%s", broker, port);
	sprintf(clientId, "SampleCV3_%s", action);

	if (opt == 0)
	{
		std::cout << "publish";
		rc = Publish(url, clientId, topic, qos, message);

	}
	else if (opt == 1)
	{
		std::cout << "subscribe";

		rc = Subscribe(url, clientId, topic, qos);
	}

}

MQTTApp::~MQTTApp()
{

}

char* MQTTApp::getMessage()
{
	return Mssg;
}

int MQTTApp::Subscribe(char* brokerUrl, char* clientId, char* topicName, int qos)
{
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;

	// Create the client instance
	rc = MQTTClient_create(&client, brokerUrl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create MQTT Client instance\n");
		return rc;
	}

	// Create the connection options
	conn_opts.keepAliveInterval = 100;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 1;

	// Connect the client
	rc = MQTTClient_connect(client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		MQTTClient_destroy(&client);
		return rc;
	}

	if (!quietMode)
		printf("Connected to %s\n", brokerUrl);

	// Subscribe to the topic
	if (!quietMode)
		printf("Subscribing to topic \"%s\" qos %d\n", topicName, qos);

	rc = MQTTClient_subscribe(client, topicName, qos);

	if (rc != MQTTCLIENT_SUCCESS)
		printf("Failed to subscribe, return code %d\n", rc);
	else
	{
		long receiveTimeout = 100l;

		while (!toStop)
		{
			char* receivedTopicName = NULL;
			int topicLen;
			MQTTClient_message* message = NULL;

			// Check to see if a message is available
			rc = MQTTClient_receive(client, &receivedTopicName, &topicLen, &message, receiveTimeout);

			if (rc == MQTTCLIENT_SUCCESS && message != NULL)
			{
				array< String^>^ dataArray = gcnew array<String^>(5);
				// A message has been received
				printf("Topic:\t\t%s\n", receivedTopicName);
				printf("Message:\t%.*s\n", message->payloadlen, (char*)message->payload);
				printf("QoS:\t\t%d\n", message->qos);
				if (count > 0)
				{
					Mssg = (char*)message->payload;
					String^ mssgString = gcnew String(Mssg);
					Console::WriteLine(mssgString);
					Char delimiter = ',';
					dataArray = mssgString->Split(delimiter);
					char mbedValuesTopic[30] = "mbed-sample/values";
					char mbedFeedbackTopic[30] = "mbed-sample/feedback";
					char * pointerValues;
					char * pointerFeedback;
					pointerValues = mbedValuesTopic;
					pointerFeedback = mbedFeedbackTopic;

					if (strcmp (receivedTopicName, pointerValues)== 0)
					{
						
						database.insertValues(dataArray[0], dataArray[1], dataArray[2], dataArray[3], dataArray[4]);
					}
					else if (strcmp(receivedTopicName, pointerFeedback)== 0)
					{					
						database.insertFeedback(dataArray[0], dataArray[1]);
					}
				}

				MQTTClient_freeMessage(&message);
				MQTTClient_free(receivedTopicName);
				printf("count%d", count);
				
				count++;

			}
			else if (rc != MQTTCLIENT_SUCCESS)
			{
				printf("Failed to received message, return code %d\n", rc);
				toStop = 1;
			}
		}
	}
	// Disconnect the client
	MQTTClient_disconnect(client, 0);

	if (!quietMode)
		printf("Disconnected\n");

	// Free the memory used by the client instance
	MQTTClient_destroy(&client);

	return rc;
}
int MQTTApp::Publish(char* brokerUrl, char* clientId, char* topicName, int qos, char* message)
{

	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_message mqttMessage = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	int rc;

	// Create the client instance
	rc = MQTTClient_create(&client, brokerUrl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create MQTT Client instance\n");
		return rc;
	}

	// Create the connection options
	conn_opts.keepAliveInterval = 20;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 1;

	// Connect the client
	rc = MQTTClient_connect(client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		MQTTClient_destroy(&client);
		return rc;
	}

	if (!quietMode)
		printf("Connected to %s\n", brokerUrl);

	// Construct the message to publish
	mqttMessage.payload = message;
	mqttMessage.payloadlen = strlen(message);
	mqttMessage.qos = qos;
	mqttMessage.retained = 0;

	// Publish the message
	if (!quietMode)
		printf("Publishing to topic \"%s\" qos %d\n", topicName, qos);

	rc = MQTTClient_publishMessage(client, topicName, &mqttMessage, &token);

	if (rc != MQTTCLIENT_SUCCESS)
		printf("Message not accepted for delivery, return code %d\n", rc);
	else
	{
		// Wait until the message has been delivered to the server
		rc = MQTTClient_waitForCompletion(client, token, 10000L);

		if (rc != MQTTCLIENT_SUCCESS)
			printf("Failed to deliver message, return code %d\n", rc);
	}

	// Disconnect the client
	MQTTClient_disconnect(client, 0);

	if (!quietMode)
		printf("Disconnected\n");

	// Free the memory used by the client instance
	MQTTClient_destroy(&client);

	return rc;	
}
