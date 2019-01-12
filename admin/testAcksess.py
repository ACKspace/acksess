import acksess

def main():
    ack = acksess.Acksess('COM5');
    users = ack.getAllUsers()

    while True:
        options = usersToOptions(users)
        choice = showMenu(['Add Ibutton'] + options)
        if choice == 99:
            break
        elif choice == 0:
            print("Add user")
        else:
            print(options[choice - 1])

    ack.close()

def showMenu(options):
    for index, option in enumerate(options):
        print("%i - %s" % (index, option))
    print("99 - Exit")
    print("")
    return int(input("Choose a option: "))

def usersToOptions(users):
    options = []
    for user in users:
        address = "%X"*6 % tuple(user[0:-1])
        options.append(address)
    return options

if __name__ == '__main__':
    main()
