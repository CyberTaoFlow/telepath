### Connection details

#### QA Telepath machine
QA Ubuntu 12 - 192.168.1.245
URL: http://192.168.1.245/telepath/
App authentication: admin/admin

#### QA client app
http://192.168.1.253/


### Test Scenario to Rule mapping

1. Leaked password list attack
    1. Access to login page directly: navigation - Navigation; Navigation (personal)
    2. Post ID and password
        1. Click speed - Click speed; Click speed (personal)
        2. Parameter length - Parameter length
    3. Repeat #1 and #2 by a script - Navigation; Navigation (personal)
2. Known password attack
    1. Access to login page from TOP page using China’s IP address and IE as client - automatic rule: Bot-Suspect
    2.
        1. Access from China. Normal users access from Japan. - Abnormal geo; Abnormal geo (personal)
        2. Browser is IE. Normal users use Firefox.  - Abnormal request attributes; Abnormal request attributes (personal)
    3. Go to cart and set quantity to 100 - Parameter anomaly. Normal users purchases 1 to 5 quantities - Abnormal request attributes; Abnormal request attributes (personal)
    4. Enter a delivery address - Parameter anomaly. The address is
different from usual one - Abnormal request attributes (personal)
3. Session hijacking
    2. Access from other client with same session ID, using China’s IP
        1. Src IP changes suddenly - Velocity, Velocity (personal)
        2. Access from China. Normal users access from Japan. - Abnormal geo; Abnormal geo (personal)
    3. Select items, go cart and set quantity to 100 - Abnormal request attributes; Abnormal request attributes (personal)
    4. Enter a delivery address - Parameter anomaly. The address is different from usual one. - Abnormal request attributes; Abnormal request attributes (personal)
4. Man In The Browser (MITB)
    3. Select items, go cart and set quantity to 100 - Parameter anomaly. Normal users purchases 1 to 5 quantities. - Abnormal request attributes; Abnormal request attributes (personal)
    4. Alter parameters, insert or delete parameters - Parameter anomaly. - Abnormal request attributes; Abnormal request attributes (personal)
5. Mass user registration by script
    2. Access to user registration page and make a user ID=dummyX@example.com - Abnormal access. Normal users tend to visit TOP page first - Navigation; Navigation (personal)
    3. Increment X and repeat #2
        1. Continuous access with similar parameters - Mass user registration or brute force
        2. ID/PW are posted instantly by a script. It takes 9 to 11 sec. for normal users. - Click speed; Click speed (personal)


> Written with [StackEdit](https://stackedit.io/).