# linux-material
Project Code is uploaded in this github

Files from here to download into your directory: 
1. `button_sim.h`
2. `button_sim.c`
3. `listener.c`
4. `trigger.c`
5. `Makefile`

Note: The directory path cannot have any spaces. 

- Step 1: Run `make`. This will create `button_sim.ko`, `listener`, and `trigger`.
- Step 2: Run `sudo insmod button_sim.ko`. (You can check that it's loaded with `lsmod | grep button_sim`)
- Step 3: Run the listener: `sudo ./listener`
- Step 4: Open a second terminal and run the trigger: `sudo ./trigger`
- Step 5: When done, unload the module: `sudo rmmod button_sim`.
