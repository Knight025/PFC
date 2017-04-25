classdef HI_Library
    %HI_LIBRARY Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        signalsArray
        signalsArraySize
    end
    
    methods
        function obj = HI_Library(signalsArraySize, signalsArray)
            obj.signalsArray = signalsArray;
            obj.signalsArraySize = signalsArraySize;
        end
    end
    
end

