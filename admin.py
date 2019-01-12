import serial

def main():
  print("0 - Add Ibutton")
  for i in range(0, len(users)):
    address = ""
    for ii in range(0,len(users[i])):
      address += "%X" % users[i][ii]
    print("%i - %s" % (i+1, address))
  print("99 - Exit")
  print("")
  # Needs validation
  userId = input("Choose a option: ")
  userId = int(userId)

  if(userId == 0):
    add()
  elif(userId == 99):
    exit()
  userId -= 1
  listUser(userId)

def add():
  print("add")

def listUser(userId):
  flags = bin(users[userId][6])[2:]
  address = ""
  for ii in range(0,len(users[i])):
    address += "%X" % users[i][ii]
  print("")
  print(address)
  print("Is adult" if flags[0] == '1' else "Is not adult")
  print("Is admin" if flags[1] == '1' else "Is not admin")

with serial.Serial('COM4', 115200) as ser:
	admin = False
	while admin == False:
		if (ser.read() == 'A'):
			admin = True
	ser.write('a')
	nRecs = ord(ser.read())
	users = []
	for i in range(0,nRecs):
		user = []
		for ii in range(0,7):
			user.append(ord(ser.read()))
		users.append(user)
	main()

"""
0 - Add Ibutton
1 - 33AE67250500
99 - Exit

Choose a option: 1

33AE67250500
Is adult
Is not admin

0 - Remove user
1 - Set child
2 - Set admin
3 - Clear secret
99 - Go to main menu

Choose a option:
"""
