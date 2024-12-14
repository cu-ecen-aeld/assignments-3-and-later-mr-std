CROSS_COMPILE =

writer: writer.c
ifeq ($(strip $(CROSS_COMPILE)),)
	echo "empty"
	cc writer.c -o writer
else
	echo "ARM compiler"
	$(CROSS_COMPILE)gcc writer.c -o writer
endif

clean: 
	rm -f writer
