
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs


.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
		pushl	%ebp
		movl	%esp, %ebp
		pushl	%ebx
		pushl 	%esi
		pushl 	%edi
		#call 	update_missiles
		#call	redraw_bases
		#call 	redraw_crosshairs
		popl	%edi
		popl	%esi
		popl	%ebx
		leave	
		ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		movl	8(%esp), %eax
		cmpl	$4, %eax			#eax - 4
		jg		invalid
		cmpl 	$0, %eax			#eax - 0
		jl 		invalid
		jmp 	*jump_table(,%eax,4)	#M[eax * 4 + jump table]

invalid:
		movl	$-1, %eax 		#set eax to -1 to return -1
		ret

jump_table:
		.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame
		


# ----------------- Functions private to this file -------------------
# int update_missiles();
# This function is a subtask of the tasklet itelf and should update the missiles
# Should erase old missile images and create new missile images flying
# Should draw explosion image 
# Inputs: none
# output: none
# Registers: standard C calling convention
update_missiles:
		
		ret

# void redraw_bases();
# This function should redraw the bases based on the status of their values ie alive or dead
# Missiles may have destroyed them or flown over so the bases will need to be updated and redrawn.
# input: none
# output: none
# registers: standard C calling convention
redraw_bases:
		ret

# void redraw_crosshairs();
# This function will redraw crosshairs based on their new position.
# It will first get the crosshair position then fill it with the character for the crosshair.
# input: none
# output: none
# registers: standard C calling convention
redraw_crosshairs:
		pushl	%ebp
		movl	%esp, %ebp
		pushl	%eax
		pushl	%ecx
		pushl	%edx
		pushl	%edi
		movl	crosshairs_y, %eax		#get xhair position by (y*80 + x)*2, 80 is width of screen
		movl	crosshairs_x, %ecx		#Each is 2 bytes so you need to mult by 2 to get the right position
		imull	%80, %eax
		addl 	%ecx, %eax
		shll	$1, %eax
		movb	$43, %cl 
		call 	mp1_poke
		popl	%edi
		popl	%edx
		popl	%ecx
		popl	%eax
		leave
		ret

# void delete_missile();
# This function should delete the missle from the game, along with deleting it from the linked list.
# it should remove the missile from the list, fix the linked list, make sure you are not deleting the head,
# then remove the missile completely so the game can continue. It should also be gone from the screen. 
# input: none
# output: none
# registers: standard C calling convention
delete_missile:
		pushl 	%ebp
		movl	%esp, %ebp
		pushl	%esi
		pushl	%ecx 
		pushl	%edx
		pushl	%edi
		movl	mp1_missile_list, %ecx
		cmpl	$0x0, %ecx
		je 		delete_head 	#if the missile list is at head jump to head removal
		

		delete_missile_loop:
				movl	NEXT(%ecx), %eax
				#cmpl	%

		delete_head:
				#movl	
		delete_end:
				popl 	%edi
				popl 	%edx
				popl	%ecx
				popl	%esi
				leave
				ret


# void mp1_ioctl_startgame(unsigned long ignore);
# Initialize the game and the variables needed for the game
# inputs: variable that is ignored
# outputs: none
# Registers: none
mp1_ioctl_startgame:
		pushl 	%ebp
		movl	%esp, %ebp
		pushl	%edi
		pushl 	%ebx
		pushl	%esi
		movl	$0, mp1_score
		movl	$0x0, mp1_missile_list
		movl 	$40, crosshairs_x
		movl	$12, crosshairs_y
		movl	$0x00111111, base_alive		#first 3 bytes are the bases and alive if non zero. set all to alive
		popl	%esi
		popl	%ebx
		popl	%edi
		leave
		ret

# int mp1_ioctl_addmissile(struct missile* user_missile);
# Should add missiles to mp1_missile_list
# inputs: pointer to the missile
# outputs: 0 if success and -1 if fail
# registers: standard C calling convention
mp1_ioctl_addmissile:
		ret

# int mp1_ioctl_movexhairs(unsigned long xhair_delta_packed);
# This function will move the cross hairs by erasing the old one 
# It will then create a new crosshair at the updated location
# input: a long int which contains the x and y value in the low 16 bits and high 16 bits respectively 
# output: 0 always as it should not fail
# registers: standard c calling convention
mp1_ioctl_movexhairs:
		pushl	%ebp
		movl	%esp, %ebp
		pushl	%eax
		pushl	%ecx
		pushl	%edx
		pushl	%edi
		pushl	%ebx
		movl	crosshairs_y, %eax		#get xhair position by (y*80 + x)*2, 80 is width of screen
		movl	crosshairs_x, %ecx		#Each is 2 bytes so you need to mult by 2 to get the right position
		imull	%80, %eax
		addl 	%ecx, %eax
		shll	$1, %eax
		movb	$32, %cl				#put space into cl to erase crosshair 
		call 	mp1_poke
		movl	$8(%ebp), %ebx
		movw	%bx, %ecx				#movw or movl? also putting the low 16 bits into ecx and high 16 bits into eax
		addl 	crosshairs_x, %ecx
		sarl	$16, %ebx				#ecx contains x, eax contains y
		movw	%bx, %eax
		addl	%crosshairs_y, %eax
		cmpl	$0, %ecx
		jl		less_x
		cmpl	$79, %ecx				#check with 0 and 79 as they are the limits
		jg		greater_x
		jmp 	y_check

		less_x:
				movl 	$0, %ecx		#if past the limits set to the limits i.e. 0/79 for x or 0/24 for y
				jmp		y_check
		greater_x:
				movl	$79, %ecx
		y_check:
				cmpl	$0, %eax
				jl		less_y
				cmpl	$24, %eax
				jg		greater_y
				movl	%eax, crosshairs_y
		less_y:
				movl	$0, %ecx
				jmp		finish_movecheck
		greater_y:
				movl	$24, %ecx
		finish_movecheck:
				movl	%ecx, crosshairs_x
				movl	%eax, crosshairs_y
				imull	%80, %eax			#get xhair position by (y*80 + x)*2
				addl 	%ecx, %eax
				shll	$1, %eax
				movb	$43, %cl
				call 	mp1_poke
				popl	%ebx
				popl	%edi
				popl	%edx
				popl	%ecx
				popl	%eax
				leave
				ret

# int mp1_ioctl_getstatus(unsigned long* user_status);
# Should receive the current score and the status of the 3 cities.
# input: a pointer to the 32 bit int in user space
# output: 0 if success and -1 if fail
# registers: standard c calling convention
mp1_ioctl_getstatus:
		push 	%ebp
		movl	%esp, %ebp
		pushl	%ebx
		pushl	%ecx
		pushl	%edi
		pushl	%esi
		pushl	%edx
		movl	8(%ebp), %edx		#edx for user status, ecx for register with score, ebx for register with score and base status
		movl	mp1_score, %ecx
		movl	%ecx, %ebx
		movb	base_alive, %eax	#store first byte aka city into eax
		cmpl	$0, %eax 			#check if city is dead
		je		next_city
		addl	$0x00010000, %ebx

		#add the values of 0x10000, 0x20000, 0x40000 to get 1 bits into the proper positions of 16, 17, 18

		next_city:
				movb	base_alive + 1, %eax	#store the next byte into EAX for the calculations
				cmpl	$0, %eax
				je 		last_city
				addl	$0x00020000, %ebx
		last_city:
				movb	base_alive + 2, %eax
				cmpl	$0, %eax
				je 		copy
				addl	$0x00040000, %ebx
		copy:
				pushl	$4					#put the proper values into the stack before calling mp1_copy_to_user
				pushl	%ebx
				pushl	%edx
				call	mp1_copy_to_user
				addl	$12, %esp			#move stack pointer back down to proper position
				cmpl	$0, %eax	
				je 		done
				movl 	$-1, %eax
		done:
				popl 	%edx
				popl	%esi
				popl	%edi
				popl 	%ecx
				popl	%ebx
				leave
				ret

# int mp1_ioctl_endgame(unsigned long ignore);
# Used when game is over to clean up work.
# Should clear linked list, free memory, and make sure there are no issues with exiting.
# input: ignored
# output: none
# registers: standard c calling convention
mp1_ioctl_endgame:
		push 	%ebp
		movl	%esp, %ebp
		pushl 	%edi
		pushl	%ebx
		pushl	%esi

		endgame_loop:
				#cmpl 	$0x0, mp1_missile_list
				#je		endgame_end
				#jmp		delete_missile
				#jmp 	endgame_loop

		endgame_end:
				popl 	%esi
				popl	%ebx
				popl	%edi
				leave
				ret



