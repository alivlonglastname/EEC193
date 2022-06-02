import AWSIoTPythonSDK.MQTTLib as AWS
import traceback
import csv
import io
import boto3
from PIL import Image
import time

compare = 0
enrolled_faces = ['ali.png']
class streaming:
    def __init__(self, name):
        self.myClient = None
        self.client_id = name
        self.enable = 1
        self.time = None
        self.cd = False

    def aws_connect(self):
        ENDPOINT = "a3k31esjy3lt07-ats.iot.us-east-1.amazonaws.com"
        PATH_TO_CERT = "c735b19bb7334164982b214d8656c8fcdd29397fe3b1dc1bc5a8239fe50649e1-certificate.pem.crt"
        PATH_TO_KEY = "c735b19bb7334164982b214d8656c8fcdd29397fe3b1dc1bc5a8239fe50649e1-private.pem.key"
        PATH_TO_ROOT = "AmazonRootCA1.pem"


        self.myClient = AWS.AWSIoTMQTTClient(self.client_id)
        self.myClient.configureEndpoint(ENDPOINT, 8883)
        self.myClient.configureCredentials(PATH_TO_ROOT, PATH_TO_KEY, PATH_TO_CERT)
        self.myClient.configureAutoReconnectBackoffTime(1, 32, 20)
        self.myClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
        self.myClient.configureDrainingFrequency(2)  # Draining: 2 Hz
        self.myClient.configureConnectDisconnectTimeout(10)  # 10 sec
        self.myClient.configureMQTTOperationTimeout(5)  # 5 sec
        try:
            if self.myClient.connect():
                print("Connected!")
                self.myClient.subscribe("esp32/cam_0", 0, callbackHandler)
                self.myClient.subscribe("esp32/enrollFace", 0, callbackHandler)
                print("Subscribed!")
            else:
                print("Not connected :(")
        except Exception as e:
            tb = traceback.format_exc()
            print(f'An error occured, Error:', e, tb)

    def recognition_message(self, payload):

        if self.enable == 1:
            self.myClient.publish("recognition/response", payload, 1)
        # internal sleep, to stop the device from recognizing the same face multiple
        # times and toggling the lock multiple times. Easier to do it on this side than on microcontroller side
        if payload == 1 and self.enable == 1:
            self.time = time.perf_counter()
            self.cd = True

        if self.cd:
            if time.perf_counter() - self.time > 10:
                self.enable = 1
                self.cd = False
            else:
                self.enable = 0


with open('credentials.csv', 'r') as input:
    next(input)
    reader = csv.reader(input)
    for line in reader:
        access_key_id = line[2]
        secret_access_key = line[3]

    # Start session to send image to s3 bucket
session = boto3.Session(
    aws_access_key_id=access_key_id,
    aws_secret_access_key=secret_access_key
)
s3 = session.resource('s3')
my_bucket = s3.Bucket("ali-alshaikh")
item_count = 0
for my_bucket_object in my_bucket.objects.all():
    item_count+=1

def callbackHandler (client, userdata, message):
    global compare
    global item_count
    im_bytes = io.BytesIO(message.payload)
    image = Image.open(im_bytes)
    image.save("img.png")
    if message.topic == 'esp32/enrollface':
        print("enrollingface")
        temp = str(item_count) + '.png'
        item_count+=1
        my_bucket.upload_file('img.png', temp)
    print("image recieved")
    compare = 1

def recognition (target):
    # Extracting user credentials
    matchFound = 0
    with open('credentials.csv', 'r') as input:
        next(input)
        reader = csv.reader(input)
        for line in reader:
            access_key_id = line[2]
            secret_access_key = line[3]

    my_bucket.upload_file(target, 'img.png')

    # start client for rekognition
    client = boto3.client('rekognition',
                        region_name='us-west-1',
                        aws_access_key_id = access_key_id,
                        aws_secret_access_key = secret_access_key)

    # compare uploaded picture
    for my_bucket_object in my_bucket.objects.all():
        if my_bucket_object.key == 'img.png':
            continue
        response = client.compare_faces(
                 SourceImage={
                     'S3Object':{
                         'Bucket': 'ali-alshaikh',
                         'Name': my_bucket_object.key
                     }
                 },
                 TargetImage = {
                     'S3Object':{
                         'Bucket':'ali-alshaikh',
                         'Name': 'img.png'
                     }
                 }
        )

        for match in response['FaceMatches']:
            if (match):
                matchFound = True
                return matchFound
        for match in response['UnmatchedFaces']:
            if (match):
                matchFound = False

    return matchFound

stream1 = streaming("esp32cam")
stream1.aws_connect()
loopcount = 0

while True:
    if compare == 1:
        print("comparing")
        try:
            val = recognition('img.png')
        except:
            val = False
        compare = 0
        if (val):
            stream1.recognition_message(1)
        else:
            stream1.recognition_message(0)
    loopcount += 1


