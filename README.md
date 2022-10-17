<style>
	.li-use {
		font-size: 1.1em;
		font-weight: 650;
	}
			
	details > summary {
		cursor: pointer;
		font-size: 1.2em;
	}
			
	#ul-notes {
		list-style-type: square;
	}
</style>

# General Info
Automatically backup files on an external hard drive on insertion.
	
## How to use
1. Installation. {.li-use}
    1. [Download](https://github.com/ItaiShek/AutoBackup/releases/download/v1.0.0/AutoBackup.zip) the compressed file.
    2. Extract it to the desired directory.
	
		
2. Creating a backup list. {.li-use}
    1. Open `BackupList.txt`.
	2. Enter each file/directory you want to back up in a new line.	
	<br>For example:
	```
	C:\Users\MyUser\Documents\Passwords.kdbx
	C:\Users\MyUser\Documents\Recipes\Krabby Patty.doc
	C:\Cat Photos\
	```

3. Creating a devices list. {.li-use}
    1. Open `VolumeList.txt`.
    2. Connect your external devices.
	3. Open the terminal and run the command `vol X:` where X is your external device.
	<br>For example:
	```
	C:\>vol E:
	 Volume in drive E is SSD ExternalHD
	 Volume Serial Number is 201A-5BA1
	```	
	4. Copy each serial number in a new line. Make sure you remove the hyphen and add a "0x" prefix.
	5. After each serial number add a semicolon and add the path to the directory on the device (`SN;PATH`).
	<br>For example:
	```	
	0x404329D6;X:\Backup
	0xad12cb34;X:\Important
	1078143446;X:\Tools\KeePassXC
	```
4. Creating a scheduled task. {.li-use}
	1. Open the task scheduler (Click ⊞ Win+R, then type taskschd.msc and press enter).
	2. Optional: Create a new folder named `My Tasks`.
	3. Create a basic task by clicking `right mouse key > Create Basic Task...` or clicking in the menu `Action > Create Basic Task...`
        1. Enter a name and a description and click `Next`.
		2. Set the trigger to `When I log on`.
		3. Select `Start A Program` in Action and click next.
		4. In the `Program/Script:` field browse and choose `AutoBackup.exe` file.<br>
		In the `Start in` field choose the directory that holds the program's files (**Important**).
		5. Optional: In the `Add arguments` field you can add the value `nocopy` if you don't want to create another copy (see [notes](#notes)). {#args}

<details>
	<summary id="notes">Notes</summary>
	<ul id="ul-notes">
		<li>The files <code>VolumeList.txt</code> and <code>BackupList.txt</code> are read <b>once</b> on program startup.</li>
		<li>The program will start the backup process when the device is inserted.</li>
		<li>If the backup directory specified doesn't exists on the external device the program will not create it, and the backup process will stop.</li>		
		<li>If the backup file already exists on your external device, then the program will copy it to the same directory with the prefix "original_", and overwrite the backup file with a new copy of the system's backup file.
		<br>
		This is done in order to prevent overwriting the backup file with a corrupted file.
		<br>
		If you don't want to create another copy (recommended for large files/directories)add the argument <code>nocopy</code> in the <a href="#args">task properties</a> (Action tag).
		</li>
		<li>The program will overwrite the backup file only if it's older than the file on the system.</li>
		<li>You can add the serial number as an integer (without <code>0x</code> prefix)</li>
	</ul>
</details>
