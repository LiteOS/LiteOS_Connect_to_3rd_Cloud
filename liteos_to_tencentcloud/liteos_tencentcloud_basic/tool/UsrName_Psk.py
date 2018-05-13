#!/bin/env python

import paho.mqtt.client as mqtt
import base64
import hashlib
import hmac
import random
import string
import time

proID   = raw_input("Please Input the product ID:")
devName = raw_input("Please Input the device name:")
psk     = raw_input("Please Input the password:")

clientid = proID+devName
username = clientid+";21010406;12365;{}".format(int(time.time()) + 36000)

def hmacsha1(content, pks):
    psk_byte = base64.b64decode(psk)
    return hmac.new(psk_byte, content, digestmod=hashlib.sha1).hexdigest()

sign = hmacsha1(username, psk)

print
print("Client ID")
print(clientid)
print("User Name:")
print(username)
print("Password:")
print(sign+";hmacsha1")

