function varargout = HI_GUI(varargin)
% HI_GUI MATLAB code for HI_GUI.fig
%      HI_GUI, by itself, creates a new HI_GUI or raises the existing
%      singleton*.
%
%      H = HI_GUI returns the handle to a new HI_GUI or the handle to
%      the existing singleton*.
%
%      HI_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in HI_GUI.M with the given input arguments.
%
%      HI_GUI('Property','Value',...) creates a new HI_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before HI_GUI_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to HI_GUI_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help HI_GUI

% Last Modified by GUIDE v2.5 09-Mar-2017 15:12:13

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @HI_GUI_OpeningFcn, ...
                   'gui_OutputFcn',  @HI_GUI_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before HI_GUI is made visible.
function HI_GUI_OpeningFcn(hObject, ~, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to HI_GUI (see VARARGIN)

% Choose default command line output for HI_GUI
handles.output = hObject;

% Load and set background image
load datos;
image(RealidadAumentada_Tacto2);
set(gca,'XtickLabel',[],'YtickLabel',[]);

% Create a new MCU-link object
DevLink = c2000Link(115200);
handles.DevLink = DevLink;

% Define boolean variables
global connected;
connected = 0;
global libraryLoaded;
libraryLoaded = 0;
global playListLoaded;
playListLoaded = 0;

% Configure a timer to keep track of the stimulation signal duration. This
% is meant to commute 'startStop' button between stances
startStopTimer = timer('TimerFcn', {@(x, y)''}, 'StopFcn', {@startStopTimer_stopFcn, hObject, handles});
handles.startStopTimer = startStopTimer;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes HI_GUI wait for user response (see UIRESUME)
% uiwait(handles.HI_GUI_figure);



% --- Outputs from this function are returned to the command line.
function varargout = HI_GUI_OutputFcn(~, ~, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



% --- Executes when user attempts to close HI_GUI_figure.
function HI_GUI_figure_CloseRequestFcn(hObject, ~, handles)
% hObject    handle to HI_GUI_figure (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: delete(hObject) closes the figure
handles.DevLink.closeLink;
delete(handles.DevLink.serial);
delete(handles.startStopTimer);
delete(hObject);



% --- Executes on button press in checkbox_periodicPlayList.
function checkbox_periodicPlayList_Callback(~, ~, handles)
% hObject    handle to checkbox_periodicPlayList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set(handles.pushbutton_sendPlayList, 'String', '¡Enviar!', 'ForegroundColor', 'red', 'FontWeight', 'bold');
%update_GUI_state(hObject, eventdata, handles);
% Hint: get(hObject,'Value') returns toggle state of checkbox_periodicPlayList



% --- Executes on button press in checkbox_requestFeedback.
function checkbox_requestFeedback_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_requestFeedback (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_requestFeedback
pushbutton_startStop = handles.pushbutton_startStop;
feedbackRequest = get(hObject,'Value');
if feedbackRequest
    set(pushbutton_startStop, 'String', 'Enviar y comenzar', 'ForegroundColor', [0 0 0], 'FontWeight', 'normal');
else
    set(pushbutton_startStop, 'String', 'Comenzar', 'ForegroundColor', [0 0 0], 'FontWeight', 'normal');
end
handles.pushbutton_startStop = pushbutton_startStop;
update_GUI_state(hObject, eventdata, handles)
%handles.playListLoaded = 1;
%guidata(hObject, handles);



% --- Executes on button press in pushbutton_loadLibrary.
function pushbutton_loadLibrary_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_loadLibrary (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global libraryLoaded;
[filename,filepath] = uigetfile('*.mat', 'Seleccione la librería de señales', 'C:\Users\Santiago Real\Documents\MATLAB\projects\Haptic Interface Prototype\Librerías\');
if(filename ~= 0)
    rawdata=load([filepath filename]);
    handles.DevLink.loadLibrary(rawdata.library);
    set(handles.pushbutton_sendLibrary, 'String', '¡Enviar!', 'ForegroundColor', 'red', 'FontWeight', 'bold');
    %set(handles.pushbutton_sendLibrary, 'Enable', 'on');
    %set(handles.pushbutton_plotLibrary, 'Enable', 'on');
    %set(handles.pushbutton_plotPlayList, 'Enable', 'on');
    %set(handles.pushbutton_sendPlayList, 'Enable', 'on');
    libraryLoaded = 1;
end

guidata(hObject, handles);
update_GUI_state(hObject, eventdata, handles)



% --- Executes on button press in pushbutton_loadPlayList.
function pushbutton_loadPlayList_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_loadPlayList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global playListLoaded;
[filename,filepath] = uigetfile('*.mat', 'Seleccione la librería de señales', 'C:\Users\Santiago Real\Documents\MATLAB\projects\Haptic Interface Prototype\playList\');
if(filename ~= 0)
    rawdata=load([filepath filename]);
    handles.DevLink.loadPlayList(rawdata.playList);
    playListLoaded = 1;
end

set(handles.pushbutton_sendPlayList, 'String', '¡Enviar!', 'ForegroundColor', 'red', 'FontWeight', 'bold');
handles.DevLink.loadPlayList(rawdata.playList);

update_GUI_state(hObject, eventdata, handles)



% --- Executes on button press in pushbutton_plotLibrary.
function pushbutton_plotLibrary_Callback(~, ~, handles)
% hObject    handle to pushbutton_plotLibrary (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
figure;
handles.DevLink.plotSignals;



% --- Executes on button press in pushbutton_plotPlayList.
function pushbutton_plotPlayList_Callback(~, ~, handles)
% hObject    handle to pushbutton_plotPlayList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
figure;
handles.DevLink.plotPlayList;



% --- Executes on button press in pushbutton_sendLibrary.
function pushbutton_sendLibrary_Callback(~, ~, handles)
% hObject    handle to pushbutton_sendLibrary (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.DevLink.sendLibrary;
set(handles.pushbutton_sendLibrary, 'String', 'Enviar', 'ForegroundColor', [0 0 0], 'FontWeight', 'normal');



% --- Executes on button press in pushbutton_sendPlayList.
function pushbutton_sendPlayList_Callback(~, ~, handles)
% hObject    handle to pushbutton_sendPlayList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

periodicPlayList = get(handles.checkbox_periodicPlayList,'Value');
if periodicPlayList
    handles.DevLink.sendPlayList(1);
else
    handles.DevLink.sendPlayList(0);
end

set(handles.pushbutton_sendPlayList, 'String', 'Enviar', 'ForegroundColor', [0 0 0], 'FontWeight', 'normal');



% --- Executes on button press in pushbutton_startStop.
function pushbutton_startStop_Callback(hObject, ~, handles)
% hObject    handle to pushbutton_startStop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

feedbackRequested = get(handles.checkbox_requestFeedback,'Value');
periodicPlayList = get(handles.checkbox_periodicPlayList,'Value');
% The timer is used to keep track of the 'active' period of the stimulation
% signal

% If the timer is running means that there is a stimulation signal
% currently being outputted, so if the button is pulsed Matlab will force
% it to stop.
% In case the timer is off, it will start outputting the last sent playList
if strcmp(get(handles.startStopTimer, 'Running'), 'off')
    if periodicPlayList
        set(handles.startStopTimer, 'StartDelay', 0);
        set(handles.startStopTimer, 'ExecutionMode', 'fixedRate');
    else
        set(handles.startStopTimer, 'StartDelay', floor(handles.DevLink.playListDuration*1000)/1000);%round to milliseconds
        set(handles.startStopTimer, 'ExecutionMode', 'singleShot');
    end
    
    set(handles.pushbutton_startStop, 'String', 'Detener', 'ForegroundColor', 'red', 'FontWeight', 'bold');
    start(handles.startStopTimer);
    
    if feedbackRequested
        if periodicPlayList
            handles.DevLink.playAndSaveFeedback(1);
        else
            handles.DevLink.playAndSaveFeedback(0);
        end
    else
       handles.DevLink.start;
    end
else
    stop(handles.startStopTimer);
    handles.DevLink.stop;
end

guidata(hObject, handles);



function startStopTimer_stopFcn(obj, ~, hObject, handles)
feedbackRequested = get(handles.checkbox_requestFeedback,'Value');
if feedbackRequested
    set(handles.pushbutton_startStop, 'String', 'Enviar y comenzar', 'ForegroundColor', [0 0 0], 'FontWeight', 'normal');
else
    set(handles.pushbutton_startStop, 'String', 'Comenzar', 'ForegroundColor', [0 0 0], 'FontWeight', 'normal');
end
handles.startStopTimer = obj;  % I use this line to prevent 'handles.DevLink' issues in following calls
guidata(hObject, handles);



% --------------------------------------------------------------------
function toolbar_connect2MCU_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to toolbar_connect2MCU (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global connected;
if strcmp(handles.DevLink.serial.Status, 'closed')
    handles.DevLink.openLink;
    connected = 1;
    set(handles.text2, 'String', 'Conectado', 'ForegroundColor', [0 0.5 0]);
    set(handles.text3, 'String', strcat(num2str(handles.DevLink.serial.BaudRate), ' baud'));
end

guidata(hObject, handles);
update_GUI_state(hObject, eventdata, handles)



% --------------------------------------------------------------------
function toolbar_disconnect2MCU_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to toolbar_disconnect2MCU (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global connected;
if strcmp(handles.DevLink.serial.Status, 'open')
    handles.DevLink.closeLink;
    connected = 0;
    set(handles.text2, 'String', 'Desconectado', 'ForegroundColor', 'red');
    set(handles.text3, 'String', '---');
end
%disp('disconnect-libraryLoaded');
%handles.libraryLoaded
%disp('disconnect-playListLoaded');
%handles.playListLoaded

guidata(hObject, handles);
update_GUI_state(hObject, eventdata, handles)


% Update each commutable button's Enable-Disable state
function update_GUI_state(hObject, ~, handles)
global libraryLoaded playListLoaded connected;;
feedbackRequested = get(handles.checkbox_requestFeedback,'Value');

if libraryLoaded
    set(handles.pushbutton_plotLibrary, 'Enable', 'on');
else
    set(handles.pushbutton_plotLibrary, 'Enable', 'off');
end

% Button sendLibrary
if connected && libraryLoaded
    set(handles.pushbutton_sendLibrary, 'Enable', 'on');
else
    set(handles.pushbutton_sendLibrary, 'Enable', 'off');
end

% Button plotPlayList
if playListLoaded
    set(handles.pushbutton_plotPlayList, 'Enable', 'on');
else
    set(handles.pushbutton_plotPlayList, 'Enable', 'off');
end

% Button sendPlayList
if connected && libraryLoaded && playListLoaded && (feedbackRequested == 0)
    set(handles.pushbutton_sendPlayList, 'Enable', 'on');
else
    set(handles.pushbutton_sendPlayList, 'Enable', 'off');
end

% CheckBoxes requestFeedback and periodicPlayList
if connected && libraryLoaded && playListLoaded
    set(handles.checkbox_requestFeedback, 'Enable', 'on');
    set(handles.checkbox_periodicPlayList, 'Enable', 'on');
else
    set(handles.checkbox_requestFeedback, 'Enable', 'off');
    set(handles.checkbox_periodicPlayList, 'Enable', 'off');
end

% Button startStop
if connected && libraryLoaded && playListLoaded
    set(handles.pushbutton_startStop, 'Enable', 'on');
else
    set(handles.pushbutton_startStop, 'Enable', 'off');
end

%disp('update_GUI_state-playListLoaded1:');
%handles.playListLoaded
guidata(hObject, handles);
%disp('update_GUI_state-playListLoaded2:');
%handles.playListLoaded
