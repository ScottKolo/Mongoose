% A simple demo to demonstrate Mongoose. Reads in a matrix, sanitizes it,
% and partitions it several different ways.
function mongoose_demo

% Obtain the adjacency matrix
matfile_data = matfile('494_bus.mat');
Prob = matfile_data.Problem;
A = Prob.A;
[m ~] = size(A);

% Sanitize the adjacency matrix: remove diagonal elements, make edge weights 
% positive, and make sure it is symmetric. If the matrix is not symmetric 
% or square, a symmetric matrix (A+A')/2 is built.
A = sanitize(A);

% Create a node weight vector and create a heavy node
V = ones(1,m);
V(10) = 300;

% Create a set of default options and modify the target balance
O = defaultoptions();
O.targetSplit = 0.3;

% Run Mongoose to partition the graph with edge and node weights.
partNode = edgecut(A, O, V);

fprintf('\n\nPartitioning graph with edge and node weights\n\n');
fprintf('=== Cut Info ===\n');
fprintf('Cut Size:   %d\n', full(sum(partNode .* sum(sign(A)))));
fprintf('Cut Weight: %d\n\n', full(sum(partNode .* sum(A))));
fprintf('=== Balance Info ===\n');
fprintf('Target Split:     0.3\n');
fprintf('Actual Split:     %1.4f\n', sum(partNode .* V) / sum(V));
fprintf('Unweighted Split: %1.4f\n', sum(partNode) / m);

% Run Mongoose to partition the graph with no vertex weights.

partEdge = edgecut(A, O);

fprintf('\n\nPartitioning graph with only edge weights\n\n');
fprintf('=== Cut Info ===\n');
fprintf('Cut Size:   %d\n', full(sum(partEdge .* sum(sign(A)))));
fprintf('Cut Weight: %d\n\n', full(sum(partEdge .* sum(A))));
fprintf('=== Balance Info ===\n');
fprintf('Target Split: 0.5\n');
fprintf('Actual Split: %1.4f\n', sum(partEdge) / m);

% Remove edge weights
A = sanitize(A, 1);

% Run Mongoose to partition the graph with no edge weights.
% Note that only the graph is passed as an argument, so default
% options are assumed.
partPattern = edgecut(A);

fprintf('\n\nPartitioning graph with only edge weights\n\n');
fprintf('=== Cut Info ===\n');
fprintf('Cut Size:   %d\n', full(sum(partPattern .* sum(sign(A)))));
fprintf('Cut Weight: %d\n\n', full(sum(partPattern .* sum(A))));
fprintf('=== Balance Info ===\n');
fprintf('Target Split: 0.5\n');
fprintf('Actual Split: %1.4f\n', sum(partPattern) / m);

figure('Position', [100, 100, 1000, 400]);

% Plot the original matrix before permutation
subplot(1, 2, 1);
spy(A)
title('Before Partitioning')

% Plot the matrix after the permutation
subplot(1, 2, 2);
perm = [find(partEdge) find(1-partEdge)];
A_perm = A(perm, perm); % Permute the matrix
spy(A_perm)
title('After Partitioning')

% Set overall title
suptitle('HB/494\_bus')

end