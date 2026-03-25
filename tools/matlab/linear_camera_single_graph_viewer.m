function linear_camera_single_graph_viewer(portName)
% Live viewer for RUNTIME_TEST_LINEAR_CAMERA UART telemetry.
% Single-graph layout: raw, filtered, gradient, thresholds, edges, and stats
% all overlaid into one axes using dual Y axes.
% Example:
%   linear_camera_single_graph_viewer("COM6")

if nargin < 1
    portName = "COM6";
end

baudRate = 115200;
sampleCount = 124;
maxEdgeCount = 12;
packetLength = 10 + 3 * sampleCount + 9 + 6 + 4 * maxEdgeCount + 1;
sync = uint8([hex2dec('A5') hex2dec('5A')]);

s = serialport(portName, baudRate, "Timeout", 1);
configureTerminator(s, "CR/LF");
flush(s);

fig = figure("Name", "Vision Debug Data", "NumberTitle", "off", ...
             "Color", "w", ...
             "CloseRequestFcn", @(src, evt)closeViewer(src, s), ...
             "KeyPressFcn", @(src, evt)toggleFreezeOnKey(src, evt));
ax = axes(fig);
fig.UserData.isFrozen = false;
fig.UserData.freezeButton = uicontrol(fig, "Style", "togglebutton", ...
    "String", "Freeze", ...
    "Units", "pixels", ...
    "Position", [12 12 80 28], ...
    "Callback", @(btn, evt)toggleFreezeButton(btn, fig));
hold(ax, "on");
grid(ax, "on");
xlim(ax, [0 sampleCount - 1]);
xlabel(ax, "Pixel");

yyaxis(ax, "left");
ylim(ax, [0 255]);
ylabel(ax, "Signal (0..255)");
hRaw = plot(ax, 0:sampleCount-1, zeros(1, sampleCount), "LineWidth", 1.2, ...
            "Color", [0.00 0.35 0.80], "LineStyle", "-", "DisplayName", "Raw");
hFiltered = plot(ax, 0:sampleCount-1, zeros(1, sampleCount), "LineWidth", 1.4, ...
                 "Color", [0.90 0.45 0.10], "LineStyle", "-", "DisplayName", "Filtered");
hLeft = xline(ax, 0, "-", "L", "Color", [0.85 0.20 0.20], "Visible", "off");
hRight = xline(ax, 0, "-", "R", "Color", [0.20 0.60 0.20], "Visible", "off");
hFinishLeft = xline(ax, 0, ":", "FgL", "Color", [0.55 0.10 0.55], "Visible", "off");
hFinishRight = xline(ax, 0, ":", "FgR", "Color", [0.55 0.10 0.55], "Visible", "off");

yyaxis(ax, "right");
ylim(ax, [-130 130]);
ylabel(ax, "Gradient (-127..127)");
hGrad = plot(ax, 0:sampleCount-1, zeros(1, sampleCount), "LineWidth", 1.0, ...
             "Color", [0.10 0.55 0.30], "DisplayName", "Gradient");
hWeakPos = yline(ax, 0, "--", "Weak+", "Color", [0.75 0.65 0.10]);
hWeakNeg = yline(ax, 0, "--", "Weak-", "Color", [0.75 0.65 0.10]);
hStrongPos = yline(ax, 0, "-", "Strong+", "Color", [0.80 0.15 0.15]);
hStrongNeg = yline(ax, 0, "-", "Strong-", "Color", [0.80 0.15 0.15]);
hEdgesPos = scatter(ax, nan, nan, 28, [0.85 0.20 0.20], "filled", "DisplayName", "Rising edges");
hEdgesNeg = scatter(ax, nan, nan, 28, [0.20 0.20 0.80], "filled", "DisplayName", "Falling edges");
hideFromLegend(hLeft);
hideFromLegend(hRight);
hideFromLegend(hFinishLeft);
hideFromLegend(hFinishRight);
hideFromLegend(hWeakPos);
hideFromLegend(hWeakNeg);
hideFromLegend(hStrongPos);
hideFromLegend(hStrongNeg);

title(ax, "Waiting for camera frames");
legend(ax, [hRaw, hFiltered, hGrad, hEdgesPos, hEdgesNeg], ...
       {"Raw", "Filtered", "Gradient", "Rising edges", "Falling edges"}, ...
       "Location", "northwest");
hText = text(ax, 0.01, 0.99, "", "Units", "normalized", ...
             "VerticalAlignment", "top", "HorizontalAlignment", "left", ...
             "FontName", "Consolas", "FontSize", 10, ...
             "BackgroundColor", [1 1 1], "Margin", 6);

rx = uint8([]);

while isvalid(fig)
    available = s.NumBytesAvailable;
    if available > 0
        newBytes = read(s, available, "uint8");
        rx = [rx; newBytes(:)]; %#ok<AGROW>
    else
        pause(0.01);
        drawnow limitrate;
        continue;
    end

    latestFrame = [];

    while numel(rx) >= packetLength
        syncIdx = find(rx(1:end-1) == sync(1) & rx(2:end) == sync(2), 1, "first");
        if isempty(syncIdx)
            rx = rx(max(1, end - 1):end);
            break;
        end

        if numel(rx) < (syncIdx + packetLength - 1)
            rx = rx(syncIdx:end);
            break;
        end

        packet = rx(syncIdx:(syncIdx + packetLength - 1));
        rx = rx((syncIdx + packetLength):end);

        if bitxor_reduce(packet(3:end-1)) ~= packet(end)
            continue;
        end

        latestFrame = parsePacket(packet, sampleCount, maxEdgeCount);
    end

    if ~isempty(latestFrame) && ~fig.UserData.isFrozen
        yyaxis(ax, "left");
        set(hRaw, "YData", double(latestFrame.raw));
        set(hFiltered, "YData", double(latestFrame.filtered));
        setMaybeXLine(hLeft, latestFrame.leftIdx, sampleCount);
        setMaybeXLine(hRight, latestFrame.rightIdx, sampleCount);
        setMaybeXLine(hFinishLeft, latestFrame.finishGapLeftIdx, sampleCount);
        setMaybeXLine(hFinishRight, latestFrame.finishGapRightIdx, sampleCount);

        yyaxis(ax, "right");
        set(hGrad, "YData", double(latestFrame.gradient));
        [weakLevel, strongLevel] = gradientThresholdLevels(latestFrame);
        set(hWeakPos, "Value", weakLevel);
        set(hWeakNeg, "Value", -weakLevel);
        set(hStrongPos, "Value", strongLevel);
        set(hStrongNeg, "Value", -strongLevel);

        [edgeXPos, edgeYPos, edgeXNeg, edgeYNeg] = edgeScatterPoints(latestFrame);
        set(hEdgesPos, "XData", edgeXPos, "YData", edgeYPos);
        set(hEdgesNeg, "XData", edgeXNeg, "YData", edgeYNeg);

        title(ax, sprintf("Vision Debug Data | frame %u", latestFrame.seq));
        hText.String = buildCompactStatsText(latestFrame);
        drawnow limitrate;
    end
end
end

function frame = parsePacket(packet, sampleCount, maxEdgeCount)
frame.seq = typecast(uint8(packet(4:5)), "uint16");
frame.confidence = packet(6);
frame.status = packet(7);
frame.errorPct = typecast(uint8(packet(8)), "int8");
frame.leftIdx = packet(9);
frame.rightIdx = packet(10);

base = 11;
frame.raw = packet(base:(base + sampleCount - 1));
base = base + sampleCount;
frame.filtered = packet(base:(base + sampleCount - 1));
base = base + sampleCount;
frame.gradient = typecast(uint8(packet(base:(base + sampleCount - 1))), "int8");
base = base + sampleCount;

frame.contrast = typecast(uint8(packet(base:(base + 1))), "uint16");
base = base + 2;
frame.maxAbsGradient = typecast(uint8(packet(base:(base + 1))), "uint16");
base = base + 2;
frame.edgeHighThreshold = typecast(uint8(packet(base:(base + 1))), "uint16");
base = base + 2;
frame.edgeLowThreshold = typecast(uint8(packet(base:(base + 1))), "uint16");
base = base + 2;

frame.splitPoint = packet(base); base = base + 1;
frame.finishGapLeftIdx = packet(base); base = base + 1;
frame.finishGapRightIdx = packet(base); base = base + 1;
frame.laneWidth = packet(base); base = base + 1;
frame.expectedFinishGap = packet(base); base = base + 1;
frame.measuredFinishGap = packet(base); base = base + 1;
frame.edgeCount = packet(base); base = base + 1;

frame.edgeIdx = nan(1, maxEdgeCount);
frame.edgePolarity = zeros(1, maxEdgeCount);
frame.edgeStrength = zeros(1, maxEdgeCount);

for i = 1:maxEdgeCount
    idx = packet(base); base = base + 1;
    polarity = typecast(uint8(packet(base)), "int8"); base = base + 1;
    strength = typecast(uint8(packet(base:(base + 1))), "uint16"); base = base + 2;

    if idx < sampleCount
        frame.edgeIdx(i) = double(idx);
    end
    frame.edgePolarity(i) = double(polarity);
    frame.edgeStrength(i) = double(strength);
end
end

function textOut = buildCompactStatsText(frame)
textOut = sprintf([ ...
    'Steering\n' ...
    'status:%s  C:%3d  E:%4d\n' ...
    '\n' ...
    'Camera\n' ...
    'Ctr:%4d  Grad:%4d  Th:%3d/%3d\n' ...
    '\n' ...
    'Line Metrics\n' ...
    'track:%3d/%3d\n' ...
    'split:%3d  width:%3d  edges:%2d\n' ...
    '\n' ...
    'Finish Line Metrics\n' ...
    'finish gap:%3d/%3d\n' ...
    'exp/meas:%3d/%3d'], ...
    statusToString(frame.status), ...
    double(frame.confidence), ...
    double(frame.errorPct), ...
    double(frame.contrast), ...
    double(frame.maxAbsGradient), ...
    double(frame.edgeHighThreshold), ...
    double(frame.edgeLowThreshold), ...
    double(frame.leftIdx), ...
    double(frame.rightIdx), ...
    double(frame.splitPoint), ...
    double(frame.laneWidth), ...
    double(frame.edgeCount), ...
    double(frame.finishGapLeftIdx), ...
    double(frame.finishGapRightIdx), ...
    double(frame.expectedFinishGap), ...
    double(frame.measuredFinishGap));
end

function out = bitxor_reduce(data)
out = uint8(0);
for i = 1:numel(data)
    out = bitxor(out, data(i));
end
end

function textOut = statusToString(status)
switch status
    case 0
        textOut = "LOST";
    case 1
        textOut = "BOTH";
    case 2
        textOut = "LEFT";
    case 3
        textOut = "RIGHT";
    otherwise
        textOut = "UNK";
end
end

function setMaybeXLine(h, idx, sampleCount)
if idx < sampleCount
    set(h, "Value", double(idx), "Visible", "on");
else
    set(h, "Visible", "off");
end
end

function [weakLevel, strongLevel] = gradientThresholdLevels(frame)
if frame.maxAbsGradient == 0
    weakLevel = 0;
    strongLevel = 0;
    return;
end

weakLevel = double(frame.edgeLowThreshold) * 127.0 / double(frame.maxAbsGradient);
strongLevel = double(frame.edgeHighThreshold) * 127.0 / double(frame.maxAbsGradient);
weakLevel = min(127, max(0, weakLevel));
strongLevel = min(127, max(0, strongLevel));
end

function [edgeXPos, edgeYPos, edgeXNeg, edgeYNeg] = edgeScatterPoints(frame)
validCount = min(double(frame.edgeCount), numel(frame.edgeIdx));
edgeXPos = [];
edgeYPos = [];
edgeXNeg = [];
edgeYNeg = [];

if frame.maxAbsGradient == 0
    scaleAbs = 1.0;
else
    scaleAbs = double(frame.maxAbsGradient);
end

for i = 1:validCount
    if isnan(frame.edgeIdx(i))
        continue;
    end

    level = min(127.0, 127.0 * double(frame.edgeStrength(i)) / scaleAbs);
    if frame.edgePolarity(i) >= 0
        edgeXPos(end + 1) = frame.edgeIdx(i); %#ok<AGROW>
        edgeYPos(end + 1) = level; %#ok<AGROW>
    else
        edgeXNeg(end + 1) = frame.edgeIdx(i); %#ok<AGROW>
        edgeYNeg(end + 1) = -level; %#ok<AGROW>
    end
end
end

function closeViewer(fig, serialObj)
if ~isempty(serialObj) && isvalid(serialObj)
    clear serialObj;
end
delete(fig);
end

function hideFromLegend(h)
h.Annotation.LegendInformation.IconDisplayStyle = "off";
end

function toggleFreezeOnKey(fig, evt)
if strcmp(evt.Key, "space") || strcmp(evt.Key, "f")
    fig.UserData.isFrozen = ~fig.UserData.isFrozen;
    syncFreezeButton(fig);
end
end

function toggleFreezeButton(btn, fig)
fig.UserData.isFrozen = logical(btn.Value);
syncFreezeButton(fig);
end

function syncFreezeButton(fig)
if fig.UserData.isFrozen
    fig.UserData.freezeButton.String = "Resume";
    fig.UserData.freezeButton.Value = 1;
else
    fig.UserData.freezeButton.String = "Freeze";
    fig.UserData.freezeButton.Value = 0;
end
end
