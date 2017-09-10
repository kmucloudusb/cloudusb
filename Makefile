all : User

User:
	cd src/user_application; $(MAKE)

clean:
	cd src/user_application; $(MAKE) clean
