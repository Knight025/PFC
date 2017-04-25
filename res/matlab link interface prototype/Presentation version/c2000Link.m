classdef c2000Link < handle
    %C2000LINK Summary of this class goes here
    %   Detailed explanation goes here
    properties
        serial
        signalSize
        signalsArray
        signalsArraySize
        playList
        playListSize
        playListDuration
    end
    
    methods (Access = private)
        function sciTX(obj, x)
            L = typecast(swapbytes(uint16(length(x)+2)), 'uint8');
            fwrite(obj.serial, [L, x]);
        end
        
        function y = sciRX(obj)
            L = double(swapbytes(typecast(uint8(fread(obj.serial, 2)), 'uint16')))-2;
            y = fread(obj.serial, L)';
        end
        
        function output = requestSignal(obj, ID)
            xSignalRequest = uint8([0, 5, typecast(swapbytes(uint16(ID)), 'uint8')]);
            sciTX(obj, xSignalRequest);
            y = sciRX(obj);
            output = typecast(reshape(flipud(reshape(uint8(y), 2, [])), 1, []), 'uint16');
        end
        
    end
    
    methods
        function obj = c2000Link(BaudRate)
            %Nota: podría configurarse el tamaño de las señales en muestras
            %desde Matlab, al igual que la resolución temporal (1 us por
            %ejemplo)
            signalSize = 100; 
            obj.signalSize = signalSize;
            obj.serial = serial('COM4','BaudRate',BaudRate,'DataBits', 8);
            obj.serial.InputBufferSize = 8192;
            obj.serial.OutputBufferSize = 8192;
            %obj.playList = [1000, 1];%En el futuro debería solicitar la playList por defecto
            %obj.playListSize = 1;
            %obj.playListDuration = 0.0011; % 0.0001 = signal max duration (for now)
            
            
            %fopen(obj.serial);
            %obj.signalsArray = zeros(signalsArraySize, signalSize);
            %sendPlayList(obj, obj.playList, 1); %Default playList
            %for i = 1:signalsArraySize
            %    obj.signalsArray(i, :) = requestSignal(obj, i);
            %end
        end
        
        function openLink(obj)
            fopen(obj.serial);
        end
        
        function closeLink(obj)
            fclose(obj.serial);
        end
        
        function loadLibrary(obj, library)
            obj.signalsArray = library.signalsArray;
            obj.signalsArraySize = library.signalsArraySize;
        end
        
        function loadPlayList(obj, playList)
            obj.playList = playList;
            obj.playListSize = length(playList)/2;
            obj.playListDuration = (sum(obj.playList(1:2:end))+obj.signalSize)*0.000001;
        end
        
        function sendLibrary(obj)
            for i = 1:obj.signalsArraySize
                 sendSignal(obj, i, obj.signalsArray(i, :));
            end
        end
        
        function sendPlayList(obj , options)
            %SENDPLAYLIST Sends stimulation signal play list
            %   options:
            %    0: play and stop
            %    1: periodic play list
            %    2: play and stop, request ADC Samples from stimulation signals
            %    3: periodic play list, request ADC Samples from firsts stimulation signals

            playListOptions = uint8([0, options]);
            playListLength = typecast(swapbytes(uint16(obj.playListSize)), 'uint8');
            data = typecast(swapbytes(uint16(typecast(uint32(obj.playList(1:2:end)+256*256*256*obj.playList(2:2:end)), 'uint16'))), 'uint8');
            
            x = uint8([0, 2, playListLength, playListOptions, data]);

            %%if options == 1 || options == 3
            %%    x(end - 3) = 1;
            %%end
            %%if options == 2 || options == 3
            %%    x(4) = 1;
            %%end

            %Lastly I introduce the 'play list end' character
            %%x = uint8([x, 0, 0, 0, 0, 0, 0, 0, 0]);
            
            sciTX(obj, x);
        end
        
        
        function sendSignal(obj, ID, signal)
            obj.signalsArray(ID,:) = signal;
            x = uint8([0, 6, typecast(swapbytes(uint16(ID)), 'uint8'), typecast(swapbytes(uint16(signal)), 'uint8')]);
            sciTX(obj, x); 
        end
        
        
        function plotSignals(obj)
           nTablas = ceil(obj.signalsArraySize/10); %10 señales por figura
           for i = 1:nTablas
               puntero = 10*(i-1);
               for j = 1:min(10, obj.signalsArraySize-puntero)
                   subplot(2, 5, j);
                   plot(obj.signalsArray(puntero+j, :))
                   axis([-5 105 -500 4595])
                   title(['Signal ',num2str(puntero+j)]);
               end
               if (nTablas-i) > 0
                   figure;
               end
           end
           
        end
        
        function plotPlayList(obj)
            boardLength = sum(obj.playList(1:2:end))+obj.signalSize;
            board = zeros(1, boardLength)+2048;

            puntero = 1;
            for i = 1:obj.playListSize
                board(puntero:(puntero+obj.signalSize-1)) = obj.signalsArray( bitand(obj.playList(2+2*(i-1)), 127) , :);
                puntero = puntero + obj.playList(1+2*(i-1));
            end

            plot(linspace(0, obj.playListDuration, boardLength), board)
            title('Señal de estimulación')
            axis([0 obj.playListDuration -500 4595]);
            ylabel('DAC');
            xlabel('Segundos');
        end
        
        function playAndSaveFeedback(obj, periodic)
            %   periodicity:
            %    0: non-periodic
            %    1: periodic
            options = 2 + periodic;
            
            playList2 = obj.playList;
            playList2(2:2:end) = bitor(playList2(2:2:end), 128);
            
            obj.loadPlayList(playList2);
            sendPlayList(obj, options);
            %start(obj);

            boardLength = sum(obj.playList(1:2:end))+100;

            board = zeros(1, boardLength) + 2048;

            puntero = 1;
            
            %limit = min(30, obj.playListSize);  %Ojo con esta linea (codigo sucio)
            answerPkg = sciRX(obj);
            nSamples = length(answerPkg)/(obj.signalSize*2);
            samplesArray = zeros(nSamples, obj.signalSize*2);
            
            for i = 1:nSamples
                samplesArray(i, :) = answerPkg((1+(i-1)*obj.signalSize*2):(i*obj.signalSize*2));
                z = typecast(reshape(flipud(reshape(uint8(samplesArray(i,:)), 2, [])), 1, []), 'uint16');
                board(puntero:(puntero+obj.signalSize-1)) = z;
                puntero = puntero + obj.playList(1+2*(i-1));
            end
   
            figure;
            subplot(2, 1, 1);
            plotPlayList(obj);

            subplot(2, 1, 2);
            plot(linspace(0, obj.playListDuration, boardLength), board)
            title('Feedback')
            axis([0 obj.playListDuration -500 4595]);
            ylabel('ADC');
            xlabel('Segundos');

        end
        
        function start(obj)
           sciTX(obj, uint8([0, 4, 0, 0])); 
        end
        
        function stop(obj)
            sciTX(obj, uint8([0, 3, 0, 0])); 
        end
        
        function output = help(obj)
            sciTX(obj, uint8([0, 1, 0, 0]));
            output = char(sciRX(obj));
        end
        
        function output = requestADCSample(obj, ID)
            xSampleRequest = uint8([0, 6, typecast(swapbytes(uint16(ID)), 'uint8')]);
            sciTX(obj, xSampleRequest);
            output = char(sciRX(obj));
        end
        
    end
    
end

