
// Feel free to use it if you subscribed to this channel: https://www.youtube.com/channel/UCWE70GK4SaYz1dxMMN0TzkQ
// The tutorial video is available here https://youtu.be/xeHBKi0GLow

var timeZone="America/Winnipeg";
// get your time zone here https://developers.google.com/adwords/api/docs/appendix/codes-formats#timezone-ids


var dateTimeFormat="dd/MM/yyyy HH:mm:ss";
var logSpreadSheetId="";
var attendanceLogSheetName="attendance log";
var defaultTerminalName="headquarter";
var mainTabName="main tab";


function onOpen() {
  var ui = SpreadsheetApp.getUi();
  
  ui.createMenu('Anyboards Menu')
  
  // can be deleted after initialization
  .addItem('Initial Setup', 'initialSetup')
  .addItem('Add New UIDs', 'addNewUIDsFromAttendanceLogUiHandler')
  .addItem('Add One Selected UID', 'addOneSelectedUID')
  .addToUi();      
      
}

function addOneSelectedUID()
{
  var tabName=SpreadsheetApp.getActiveSheet().getName();
  if(tabName!=attendanceLogSheetName)
  {
    SpreadsheetApp.getUi().alert('It Shoud be '+ attendanceLogSheetName+' sheet');
  }
  var row=SpreadsheetApp.getActiveSheet().getActiveCell().getRow();
  var col=SpreadsheetApp.getActiveSheet().getActiveCell().getColumn();
  
    addNewUIDsFromAttendanceLog(row);
}

function addNewUIDsFromAttendanceLogUiHandler()
{
  var ui = SpreadsheetApp.getUi();
  var response = ui.alert('All new Uids from '+attendanceLogSheetName+' will be added to the main tab', 'Are you sure?', ui.ButtonSet.YES_NO);

// Process the user's response.
if (response == ui.Button.YES) {
  addNewUIDsFromAttendanceLog();
} 
}


function addNewUIDsFromAttendanceLog(row)
{
  
  var mainTab=getMainSheet();
  var data=mainTab.getRange(2,1,mainTab.getLastRow(),1).getValues();
  var registeredUIDs=[];
  data.forEach(x=>registeredUIDs.push(x[0]));
  
  registeredUIDs=[...new Set(registeredUIDs)]; //distinct 

  
  var attendanceSheet=getAttendanceLogSheet();
  
  var data;
  if(row)
    data=attendanceSheet.getRange(row,1,row,2).getValues();
  else
  data=attendanceSheet.getRange(2,1,attendanceSheet.getLastRow(),2).getValues();
  var arr=[];
  
  for(var i=0; i<data.length; i++)
  {
    var visit=[];
    var uid=data[i][1];
    if(!registeredUIDs.includes(uid))
       {
    
 visit.date=data[i][0];
visit.uid=uid;
         arr.push(visit)
         registeredUIDs.push(uid);
       }
  }

  var startRow=mainTab.getLastRow()+1;
  data=[];
  for(var i=arr.length-1; i>=0; i--) // in reverse for proper last visit
  {
    //'UID','Name','Access','Text','Last Visit'
    var row=[];
    row[0]=arr[i].uid;
    row[1]='Person '+(startRow-2+arr.length-i);
    row[2]=-1;
    row[3]="You're unregistered";
    row[4]=0;
    row[5]=arr[i].date;
    data.push(row);
    
  }
  if(data.length>0)
  mainTab.getRange(startRow, 1,data.length,data[0].length).setValues(data);
  
}

// creates sheets and columns
function initialSetup()
{

  if(!getAttendanceLogSheet())
  {
  var mainSheet=SpreadsheetApp.getActiveSheet().setName(mainTabName);
  var rowData = ['UID','Name','Access','Text To Display','Visits Count', 'Last Visit'];
  mainSheet.getRange(1, 1,1,rowData.length).setValues([rowData]);
  mainSheet.setColumnWidths(1, rowData.length+1,150);
  
  rowData=['Date Time','UID','Name','Result','Terminal'];
  var attendanceSheet=SpreadsheetApp.getActiveSpreadsheet().insertSheet(attendanceLogSheetName);
  attendanceSheet.getRange(1, 1,1,rowData.length).setValues([rowData]);
    attendanceSheet.setColumnWidths(1, rowData.length+1,150);
  
  }

  else{
    
    var ui = SpreadsheetApp.getUi();
  ui.alert('The spreadsheet system has already been initialized');
  }
}


function doGet(e) {
  //default values
      var access="-1";
      var text='go home';
      var name='Who are you?';
  
  var dateTime=Utilities.formatDate(new Date(), timeZone, dateTimeFormat);
  // var json;
  // var error="idk";
    Logger.log(JSON.stringify(e)); // view parameters
    var result = 'Ok'; // assume success
    if (e.parameter == 'undefined') {
        result = 'No Parameters';
    } else {

        var uid = '';
        // var onlyPing=false;
        var terminal = defaultTerminalName;
        // var error = '';
        for (var param in e.parameter) {

            var value = stripQuotes(e.parameter[param]);

            switch (param) {
                case 'uid':
                    uid = value;
                    break;
                case 'terminal':
                    terminal = value;
                    break;
       
                default:
                    result = "unsupported parameter";
            }
        }
      
     
      var mainSheet=getMainSheet();
     
    var data = mainSheet.getDataRange().getValues();
    if (data.length == 0)
        return;
        //checking if uid is known
    for (var i = 0; i < data.length; i++) {

        if (data[i][0] ==uid)
        {
          name=data[i][1];
          access=data[i][2];
          text=data[i][3];
          var numOfVisits=mainSheet.getRange(i+1,5).getValue();
          mainSheet.getRange(i+1,5).setValue(numOfVisits+1);
          mainSheet.getRange(i+1,6).setValue(dateTime+' '+terminal);
          break;
        }
       
    }
      //inserting record to attendence log
      var attendanceSheet=getAttendanceLogSheet();
      data=[dateTime,uid,name,access,terminal];
      attendanceSheet.getRange(attendanceSheet.getLastRow()+1,1,1,data.length).setValues([data]);
       }

     
  result=access+":"+name+":"+text;
  return ContentService.createTextOutput(result);

}


function getAttendanceLogSheet()
{
  return SpreadsheetApp.getActiveSpreadsheet().getSheetByName(attendanceLogSheetName);
}

function getMainSheet()
{
  return SpreadsheetApp.getActiveSpreadsheet().getSheetByName(mainTabName);
}


/**
 * Remove leading and trailing single or double quotes
 */
function stripQuotes(value) {
    return value.replace(/^["']|['"]$/g, "");
}