# drive-on-line
2+1 wheels platform with the ability to drive on the line
Hand made drive-on-line platform side view

![Drive on line platform side view](https://github.com/dbprof/drive-on-line/blob/master/view.jpg)

1. 1 x Uno R3 MCU Main Board IC MEGA328P ATMEGA16U2;
2. 1 x Motor Shield v2 Control 4 DC Motors or 2 Stepper Motors;
3. 3 x IR Infrared Line Track Follower Sensor TCRT5000;
4. 2 x DC motor + supporting wheels;
5. 1 x HC-06 Bluetooth module;
6. 1 x 4 Bits TM1637 Red Digital Tube LED Display Module (Not necessary).


As first I used the simple car android application with lots of unused functions: Arduino Bluetooth RC Car

![Bluetooth RC Car logo](https://lh6.ggpht.com/ldZIYdLWtYOuPhm9l15bdF9UAy2pefnP9EdXtMgKmoYtisezJgDpYDgGuQSY5HB6KQ=w300)

Thanks for the developers ([Link to Google Play](https://play.google.com/store/apps/details?id=braulio.calle.bluetoothRCcontroller))

The first run was so twitchy! ([Video](https://youtu.be/D4nX4GFL9H4))

The first idea was to harmonize the moving without backward. I used 4 additional function to remember the last move (setLast1...).
Then I found new application for the android remote control RemoteXY (http://remotexy.com) and used free version.

![RemoteXY logo](https://lh3.googleusercontent.com/MO7Hg3DL6StPmHLkFufg49M2Jhry1tusTNPzMhkuYv56eryvTvgsxNpjNzbDq0vpS7OR=w300)

But the signal from android can't be read by arduino properly. I found excellent idea with the Timer library (http://playground.arduino.cc/Code/Timer). 

Now it moves more graceful.([Video](https://youtu.be/IqA3M4lxxi4))

To complete the theme, I decided to make some control to driving. At the fork you can make a decision which way to proceed moving. ([Video](https://youtu.be/YkwSzJOifUs))
