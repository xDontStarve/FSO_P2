cocos0 : cocos0.c winsuport.o winsuport.h
	gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses

cocos1 : cocos1.c winsuport.o winsuport.h
	gcc -g -Wall cocos1.c winsuport.o -o cocos1 -lcurses -lpthread

cocos2 : cocos2.c winsuport.o winsuport.h
	gcc -Wall cocos2.c winsuport.o -o cocos2 -lcurses -lpthread

cocos3 : cocos3.c menjacocos3.h winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -g -Wall cocos3.c winsuport2.o memoria.o -o cocos3 -lcurses -lpthread

fantasma3: fantasma3.c menjacocos3.h winsuport2.o winsuport2.h
	gcc -g -Wall fantasma3.c winsuport2.o -o fantasma3 -lcurses -lpthread

%.o : %.c
	gcc -g -Wall -c $< -o $@

clean:
	rm -f cocos0 cocos1 cocos2 cocos3 fantasma3 winsuport.o winsuport2.o
