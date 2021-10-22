% Initialize RodStimUI, call once at the beginning
function RodStimInit

    result = RodStimUI('init','log',true,'PCM','PCM8Y9HWA');
    
    % Check if result is not success
    if ~strcmp(result,'success')
        % Display the error
        disp(result);
        % Terminate further execution
    end

end