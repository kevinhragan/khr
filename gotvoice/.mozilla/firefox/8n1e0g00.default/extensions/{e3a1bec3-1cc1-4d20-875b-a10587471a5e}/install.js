/****************************************************
 * PACKAGE INSTALLATION SCRIPT (Created 2002-2005)        
 *                                                   
 * Contributors:                                     
 *  Stephen Bounds <GuruJ@mbox.com.au>               
 *  Biju G C <bijumaillist@yahoo.com>
 *                                                   
 ****************************************************/

// 1. Set parameters of package installation
var __pkgName  = "Preferential";
var __progName = "preferential";
var __version   = "0.8.2";
var __author    = "Stephen Bounds";
var __locale    = "en-US";

// set useful constants and objects
var __jarName    = __progName + ".jar";
var __appFolder  = getFolder("Chrome", "");
var __userFolder = getFolder("Current User", "chrome");


// 2. Initialise package
var err = initInstall(__pkgName, "/" + __author + "/" + __pkgName, __version);


// 3. Check where to install
var flagLocation        = PROFILE_CHROME;
var installFolder       = __userFolder;
var existsInApplication = File.exists(getFolder(__appFolder, __jarName));
var existsInProfile     = File.exists(getFolder(__userFolder, __jarName));
var msgLocationChk      = "Select Install Location: for " + __pkgName + "\n"
                        + "Do you want to install the extension into your profile folder?\n"
                        + "  Click [OK] to install in your Personal Profile folder;\n"
                        + "  Click [Cancel] to install into the Application folder."

// If this is first install of package, ask where XPI should be installed
if(existsInApplication || (!existsInProfile && !confirm(msgLocationChk)))
{
  flagLocation  = DELAYED_CHROME;
  installFolder = __appFolder;
}


// 4. Flag files/folders to be added
addFile(__pkgName, "chrome/" + __jarName, installFolder, "");


// 5. Register chrome (this is what contents.rdf is used for)
__package = getFolder(installFolder, __jarName)

registerChrome(CONTENT | flagLocation, __package, "content/");
registerChrome(LOCALE  | flagLocation, __package, "locale/" + __locale + "/" + __progName + "/");
//registerChrome(SKIN | flagLocation, __package, "skin/");


// 6. Perform the installation
err = performInstall();


// 7. Report on success or otherwise  
if ( err == SUCCESS || err == "999" ) {
   refreshPlugins();
   alert(__pkgName + " has been installed. \n You must exit quick launch and restart your browser/application to continue.");
}
else { 
   alert("Install failed. \nError code:" + err);
   cancelInstall(err);
}