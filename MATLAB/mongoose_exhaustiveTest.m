function mongoose_exhaustiveTest
    index = UFget;

    for i = 1:length(index.nrows)
        if (index.isReal(i) && index.numerical_symmetry(i) && index.nnz(i) < 1E6)
            Prob = UFget(i);
            fprintf('Computing separator for %d: %s\n', i, Prob.name);
            A = Prob.A;

            % Sanitize the matrix: remove diagonal elements, check for positive edge
            % weights, and make sure it is symmetric.
            A = sanitize(A);

            % Run Mongoose to partition the graph.
            tic
            partition = edgecut(A);
            toc
            fprintf('\n');
        end
    end
end