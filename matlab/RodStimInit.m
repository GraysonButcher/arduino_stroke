% Initialize RodStimUI, call once at the beginning
function RodStimInit

    result = RodStimUI('init','log',true,'PCM','PCMBETEZA');
    
    % Check if result is not success
    if ~strcmp(result,'success')
        % Display the error
        disp(result);
        % Terminate further execution
    end

end