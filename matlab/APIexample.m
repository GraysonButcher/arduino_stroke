
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

% Start a stimulation
function stimulate

    result = RodStimUI('stimulate','amplitude',1,'frequency',30,'pulse width',100,'duration',5,'callback',@stimEnd);
    
    % Check if stimulation started successfully
    if strcmp(result,'success')
        % Stimulation started
    else
        % Display the error
        disp(result);
    end

end

% Receive notification of the end of stimulation
function stimEnd(result, noOfPulses)

    % Check if stimulation completed successfully
    if strcmp(result,'success')
        % Display the number of pulses delivered
        disp(noOfPulses);
    else
        % Display the error
        disp(result);
    end
    
end

% Impedance measurement
function impedanceCheck

    result = RodStimUI('impedance','amplitude',2,'pulse width',100);
    
    % Check if result is success
    if strcmp(result,'success')
        % Get the voltage data from the figure and close it
        data = get(get(get(gcf,'children'),'children'),'YData');
        close(gcf);
    else
        % Display the error
        disp(result);
    end
    
end

% Terminate RodStimUI, call once at the end
function RodStimClose

    RodStimUI('close');
    
end
