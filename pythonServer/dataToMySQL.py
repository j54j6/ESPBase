import paho.mqtt.client as mqtt
import json
import mysql.connector

mydb = mysql.connector.connect(
  host="localhost",
  user="homereport",
  password="s698r7GHwgPVala3XSVO",
  database="homestats"
)

mycursor = mydb.cursor()
def on_message(client, userdata, message):
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

client =mqtt.Client('mainServer')
client.connect("192.168.178.27", port=1883, keepalive=60, bind_address="")
client.subscribe("/data/report/temperature/terasse")
client.on_message=on_message        #attach function to callback

while (True):
    client.loop_start() #start the loop