import paho.mqtt.client as mqtt
import json
import mysql.connector

mydb = mysql.connector.connect(
  host="localhost",
  user="<<yourUser>>",
  password="<<yourPass>",
  database="<<your Tablename>"
)

mycursor = mydb.cursor()
def on_message(client, userdata, message):
  try:
    message = str(message.payload.decode("utf-8"))
    jsonParsed = json.loads(message)
    if(jsonParsed["mac"] == "2C:F4:32:51:21:83"):
      sql  = "INSERT INTO tempTerasse(temp, humid) VALUES (%s, %s)"
      val = (jsonParsed["temp"], jsonParsed["humid"])
      mycursor.execute(sql, val)
      mydb.commit()
      print(mycursor.rowcount, "record inserted.")
    else:
      print("Wrong MAC!")
  except:
    print("Can't parse JSON - SKIP!")

client =mqtt.Client('mainServer')
client.connect("192.168.178.27", port=1883, keepalive=60, bind_address="")
client.subscribe("espos/usedata/2C:F4:32:51:21:83/module/dht11Temp/temperature")
client.on_message=on_message        #attach function to callback

while (True):
    client.loop_start() #start the loop
