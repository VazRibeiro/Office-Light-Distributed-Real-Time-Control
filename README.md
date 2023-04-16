# Office-Light-Distributed-Real-Time-Control

- [x] ADC Reading
- [x] Serial Communication
- [x] PID
- [x] Anti windup
- [x] Timer interrupts
- [ ] Buffer
- [ ] Metrics
- [x] CAN
- [x] Hub mode
- [ ] Wake up
- [ ] Calibration
- [ ] Decentralized control
- [ ] TCP/IP and UDP demo clients
- [ ] Console for user commands
- [ ] Remote operation

# Communications scheme


![alt text](/images/communications.png)

## can_id
Message type | Sender | Receiver
:------------ | :-------------| :-------------
5 bits | 12 bits |  12 bits 

## List of commands
Command | Status | Response | Status
:------------ | :-------------| :-------------| :-------------
d <i> <val> | :heavy_check_mark: |  “ack” or “err” | :heavy_check_mark:
:------------ | :-------------| :-------------| :-------------
g d <i> | :heavy_check_mark: |  d <i> <val> | :heavy_check_mark:
:------------ | :-------------| :-------------| :-------------
r <i> <val> | :heavy_check_mark: |  “ack” or “err” | :heavy_check_mark:
:------------ | :-------------| :-------------| :-------------
g r <i> | :white_check_mark: |  r <i> <val> | :white_check_mark:
:------------ | :-------------| :-------------| :-------------
g l <i> | :white_check_mark: |  l <i> <val> | :white_check_mark: