cocos0 : cocos0.c winsuport.o winsuport.h
	gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses

cocos1 : cocos1.c winsuport.o winsuport.h
	gcc -g -Wall cocos1.c winsuport.o -o cocos1 -lcurses -lpthread

cocos2 : cocos2.c winsuport.o winsuport.h
	gcc -Wall cocos2.c winsuport.o -o cocos2 -lcurses -lpthread

cocos3 : cocos3.c menjacocos3.h winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -g -Wall cocos3.c winsuport2.o memoria.o -o cocos3 -lcurses -lpthread

fantasma3 : fantasma3.c menjacocos3.h winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -g -Wall fantasma3.c winsuport2.o memoria.o -o fantasma3 -lcurses -lpthread

cocos4 : cocos4.c menjacocos4.h winsuport2.o winsuport2.h memoria.o memoria.h semafor.o semafor.h
	gcc -g -Wall cocos4.c winsuport2.o memoria.o semafor.o -o cocos4 -lcurses -lpthread

fantasma4 : fantasma3.c menjacocos4.h winsuport2.o winsuport2.h memoria.o memoria.h semafor.o semafor.h missatge.o missatge.h
	gcc -g -Wall fantasma4.c winsuport2.o memoria.o semafor.o missatge.o -o fantasma4 -lcurses -lpthread -lm

run: fantasma4 cocos4
	./cocos4 joc34.txt 250

%.o : %.c
	gcc -g -Wall -c $< -o $@

clean:
	rm -f cocos0 cocos1 cocos2 cocos3 fantasma3 cocos4 fantasma4 *.o 
