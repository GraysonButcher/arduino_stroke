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