% Start a stimulation
function stimulate

    result = RodStimUI('stimulate','amplitude',1,'frequency',30,'pulse width',100,'duration',.5,'callback',@stimEnd);
    
    % Check if stimulation started successfully
    if strcmp(result,'success')
        % Stimulation started
    else
        % Display the error
        disp(result);
    end

end
