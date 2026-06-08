%% 1. Φόρτωση Δεδομένων από 2 Αρχεία
clear; clc; close all;

% Ονόματα αρχείων
file_14 = '/MATLAB Drive/Thesis/Thesis_Plots_Final/test_results_14dBm.csv';
file_22 = '/MATLAB Drive/Thesis/Thesis_Plots_Final/test_results_22dBm.csv';

% Έλεγχος αν υπάρχουν
if ~isfile(file_14) || ~isfile(file_22)
    error('Δεν βρέθηκαν τα αρχεία CSV! Ανέβασέ τα στο MATLAB Online.');
end

% Ρυθμίσεις εισαγωγής (για σιγουριά με τις επικεφαλίδες)
opts = detectImportOptions(file_14);
opts.VariableNames = {'Time', 'SenderID', 'Packet', 'RSSI', 'SNR', 'ORP'};

% Διάβασμα των αρχείων
data14 = readtable(file_14, opts);
data22 = readtable(file_22, opts);

% Δημιουργία φακέλου
if ~exist('Thesis_Plots', 'dir')
    mkdir('Thesis_Plots');
end

%% 2. Υπολογισμός Packet Loss (Ξεχωριστά για τον καθένα)

function [loss_pct, total_rx, max_dist] = calc_stats(packets)
    if isempty(packets)
        loss_pct = 0; total_rx = 0; max_dist = 0; return;
    end
    min_p = min(packets);
    max_p = max(packets);
    expected = max_p - min_p + 1;
    received = length(packets);
    
    loss_pct = ((expected - received) / expected) * 100;
    total_rx = received;
    max_dist = max_p; % Το τελευταίο πακέτο δείχνει τη μέγιστη απόσταση
end

[loss_14, rx_14, dist_14] = calc_stats(data14.Packet);
[loss_22, rx_22, dist_22] = calc_stats(data22.Packet);

fprintf('=== ΣΥΓΚΡΙΤΙΚΑ ΑΠΟΤΕΛΕΣΜΑΤΑ ===\n');
fprintf('14 dBm (Legal): Packet Loss: %.2f%% | Max Packet ID: %d\n', loss_14, dist_14);
fprintf('22 dBm (High):  Packet Loss: %.2f%% | Max Packet ID: %d\n', loss_22, dist_22);
fprintf('=================================\n');

%% 3. ΔΙΑΓΡΑΜΜΑ 1: RSSI Comparison (Το βασικό σου γράφημα)
figure('Position', [100, 100, 1000, 600]);

% Γραμμή για 14dBm (Μπλε)
plot(data14.Packet, data14.RSSI, 'b-o', 'LineWidth', 1.5, 'MarkerSize', 4, 'DisplayName', '14 dBm (Legal Limit)');
hold on;

% Γραμμή για 22dBm (Κόκκινο)
plot(data22.Packet, data22.RSSI, 'r-s', 'LineWidth', 1.5, 'MarkerSize', 4, 'DisplayName', '22 dBm (High Power)');

grid on;
title('RSSI Range Comparison: 14dBm vs 22dBm');
xlabel('Packet Sequence (Distance)');
ylabel('RSSI (dBm)');
ylim([-145 -20]);

% Γραμμή ορίου ευαισθησίας
yline(-120, 'k--', 'LoRa Sensitivity Limit (-120 dBm)', 'LineWidth', 2, 'DisplayName', 'Limit');

legend('Location', 'SouthWest');
set(gca, 'FontSize', 12);

saveas(gcf, 'Thesis_Plots/Comparative_RSSI.png');

%% 4. ΔΙΑΓΡΑΜΜΑ 2: SNR Comparison
figure('Position', [150, 150, 1000, 600]);

plot(data14.Packet, data14.SNR, 'b.', 'MarkerSize', 10, 'DisplayName', '14 dBm');
hold on;
plot(data22.Packet, data22.SNR, 'r.', 'MarkerSize', 10, 'DisplayName', '22 dBm');

grid on;
title('Signal-to-Noise Ratio (SNR) Comparison');
xlabel('Packet Sequence');
ylabel('SNR (dB)');
yline(-20, 'k--', 'Noise Floor');
legend('Location', 'Best');
set(gca, 'FontSize', 12);

saveas(gcf, 'Thesis_Plots/Comparative_SNR.png');

%% 5. ΔΙΑΓΡΑΜΜΑ 3: ORP Values (Από το δυνατό σήμα)
figure('Position', [200, 200, 1000, 600]);

plot(data22.Packet, data22.ORP, 'g-d', 'LineWidth', 1.5, 'MarkerFaceColor', 'g');
grid on;
title('Water Quality Sensor: ORP Measurements');
xlabel('Packet Sequence');
ylabel('ORP Value (mV)');
set(gca, 'FontSize', 12);

saveas(gcf, 'Thesis_Plots/ORP_Values.png');

fprintf('Έτοιμα! Τα διαγράμματα αποθηκεύτηκαν στον φάκελο Thesis_Plots.\n');