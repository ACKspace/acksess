import serial

with serial.Serial('/dev/ttyUSB0', 115200) as ser:
	admin = False
	while admin == False:
		if (ser.read() == 'A'):
			admin = True
	ser.write('a')
	nRecs = ord(ser.read())
	print nRecs
	users = []
	for i in range(0,nRecs):
		user = []
		for ii in range(0,7):
			user.append(ord(ser.read()))
		users.append(user)
	print users
