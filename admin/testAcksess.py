import acksess

ack = acksess.Acksess('COM5');
print(ack.getAllUsers())
ack.close()
