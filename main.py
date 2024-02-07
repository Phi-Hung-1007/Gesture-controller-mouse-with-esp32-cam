import numpy as np
import cv2
import pyautogui
import time
import track_hand as htm
import webbrowser
import urllib.request

url= "http://192.168.31.71/hi.jpg"
wCam, hCam = 800, 600
frameR = 100
smoothening = 20

pTime = 0
plocX, plocY = 0, 0
clocX, clocY = 0, 0

'''cap = cv2.VideoCapture(0)
cap.set(3, wCam)
cap.set(4, hCam)'''

detector = htm.handDetector(maxHands=1)
wScr, hScr = pyautogui.size()

virtual_keyboard_opened = False
mouse_click = False
say_hello = False
while True:
    fingers = [0, 0, 0, 0, 0]
    #success, img = cap.read()

    img_resp=urllib.request.urlopen(url)
    imgnp=np.array(bytearray(img_resp.read()),dtype=np.uint8)
    img=cv2.imdecode(imgnp, cv2.IMREAD_COLOR)

    img = detector.findHands(img)
    lmList, bbox = detector.findPosition(img)

    if len(lmList) != 0:
        x1, y1 = lmList[8][1:]
        x2, y2 = lmList[12][1:]
        fingers = detector.fingersUp()

    cv2.rectangle(img, (frameR, frameR), (wCam - frameR, hCam - frameR), (255, 0, 255), 2)
    if fingers[1] == 1 and fingers[2] == 0:
        virtual_keyboard_opened = False
        mouse_click = False
        x3 = np.interp(x1, (frameR, wCam - frameR), (0, wScr))
        y3 = np.interp(y1, (frameR, hCam - frameR), (0, hScr))
        clocX = plocX + (x3 - plocX) / smoothening
        clocY = plocY + (y3 - plocY) / smoothening
        pyautogui.moveTo(wScr - clocX, clocY)
        cv2.circle(img, (x1, y1), 15, (255, 0, 255), cv2.FILLED)
        plocX, plocY = clocX, clocY

    if fingers[1] == 1 and fingers[2] == 1 and fingers[3] == 0:
        length, img, lineInfo = detector.findDistance(8, 12, img)
        print(length)
        if length < 40 and not mouse_click:
            cv2.circle(img, (lineInfo[4], lineInfo[5]), 15, (0, 255, 0), cv2.FILLED)
            pyautogui.click()
            mouse_click = True

    if fingers[1] == 1 and fingers[2] == 1 and fingers[3] == 1:
        if not virtual_keyboard_opened:
            pyautogui.hotkey('win', 'r')
            pyautogui.typewrite('osk')
            pyautogui.press('enter')
            virtual_keyboard_opened = True

    cTime = time.time()
    fps = 1 / (cTime - pTime)
    pTime = cTime
    cv2.putText(img, str(int(fps)), (20, 50), cv2.FONT_HERSHEY_PLAIN, 3, (255, 0, 0), 3)

    cv2.imshow("Image", img)
    cv2.waitKey(1)
