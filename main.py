from keras.models import load_model  # TensorFlow is required for Keras to work
import cv2  # Install opencv-python
import numpy as np
import serial
import serial
import time

# Disable scientific notation for clarity
np.set_printoptions(suppress=True)

# Load the model
model = load_model("keras_model.h5", compile=False)

# Load the labels
class_names = open("labels.txt", "r").readlines()

# CAMERA can be 0 or 1 based on default camera of your computer
camera = cv2.VideoCapture(0)

ser = serial.Serial("/dev/ttyUSB0", 9600)
time.sleep(3)

buf = ""

score = 0

active = False
while True:
    if ser.in_waiting:
        char = ser.read(1)
        print(char)
        if char == b"b":
            active = True
            ser.readline()
        elif char == b"q":
            active = False
            ser.readline()
            time.sleep(5)
        else:
            score = int((char + ser.readline()).decode("utf-8"))
    
    # Grab the webcamera's image.
    ret, image = camera.read()

    # Resize the raw image into (224-height,224-width) pixels
    image = cv2.resize(image, (224, 224), interpolation=cv2.INTER_AREA)

    imgwtext = cv2.putText(image, f"{score}", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 5)

    # Show the image in a window
    cv2.imshow("Webcam Image", imgwtext)

    # Make the image a numpy array and reshape it to the models input shape.
    image = np.asarray(image, dtype=np.float32).reshape(1, 224, 224, 3)

    # Normalize the image array
    image = (image / 127.5) - 1

    # Predicts the model
    prediction = model.predict(image)
    index = np.argmax(prediction)
    class_name = class_names[index]
    confidence_score = prediction[0][index]

    # Print prediction and confidence score
    # print("Class:", class_name[2:], end="")
    # print("Confidence Score:", str(np.round(confidence_score * 100))[:-2], "%")
    if class_name == "0 Class 1\n" and not active:
        print("asdf")
        ser.write(b"a\n")

    # Listen to the keyboard for presses.
    keyboard_input = cv2.waitKey(1)

    # 27 is the ASCII for the esc key on your keyboard.
    if keyboard_input == 27:
        break

camera.release()
cv2.destroyAllWindows()

ser.close()
