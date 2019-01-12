import serial

class Acksess:
    def __init__(self, port):
        self.ser = serial.Serial(port, 115200)
        admin = False
    	while admin == False:
    		if (self.ser.read() == 'A'):
    			admin = True
        print('admin mode initialized')

    def getAllUsers(self):
        self.ser.write('a')
    	nRecs = ord(self.ser.read())
    	users = []
    	for i in range(0,nRecs):
    		user = []
    		for ii in range(0,7):
    			user.append(ord(self.ser.read()))
    		users.append(user)
        return users

    def addUser(self):
        pass

    def removeUser(self):
        pass

    def updateFlags(self):
        pass

    def clearSecret(self):
        pass

    def close(self):
        self.ser.close()
