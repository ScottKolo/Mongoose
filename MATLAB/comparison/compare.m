function [rel_mongoose_times, rel_metis_times, avg_mongoose_times, avg_metis_times] = compare
    index = UFget;
    j = 1;
    for i = 1:length(index.nrows)
        if (index.isReal(i) && index.numerical_symmetry(i) && index.nnz(i) < 1E7)
            Prob = UFget(i);
            fprintf('Computing separator for %d: %s\n', i, Prob.name);
            A = Prob.A;

            % Sanitize the matrix: remove diagonal elements, check for positive edge
            % weights, and make sure it is symmetric.
            A = mongoose_sanitizeMatrix(A);

            % Run Mongoose to partition the graph.
            for k = 1:5
                tic;
                partition = mongoose_computeEdgeSeparator(A);
                t = toc;
                fprintf('Mongoose: %0.2f\n', t);
                mongoose_times(j,k) = t;
            end
            
            for k = 1:5
                tic;
                [perm,iperm] = metispart(A);
                t = toc;
                fprintf('METIS:    %0.2f\n', t);
                metis_times(j,k) = t;
            end
            j = j + 1;
        end
    end
    
    for i = 1:length(mongoose_times)
        avg_mongoose_times(i) = max(1E-6, trimmean(mongoose_times(i,:), 40));
        avg_metis_times(i) = max(1E-6, trimmean(metis_times(i,:), 40));
        min_time = min([avg_mongoose_times(i), avg_metis_times(i)]);
        
        rel_mongoose_times(i) = (avg_mongoose_times(i) / min_time);
        rel_metis_times(i) = (avg_metis_times(i) / min_time);
        if (rel_mongoose_times(i) > 2)
            disp('outlier! Mongoose significantly worse.')
            i
        end
        if (rel_metis_times(i) > 2)
            disp('outlier! METIS significantly worse.')
            i
        end
    end
    
    rel_mongoose_times = sort(rel_mongoose_times);
    rel_metis_times = sort(rel_metis_times);
    
    plot(rel_mongoose_times, 1:length(rel_mongoose_times), 'Color', 'b');
    hold on;
    plot(rel_metis_times, 1:length(rel_metis_times), 'Color','r');
    hold off;

    [error, commit] = system('git rev-parse --short HEAD');
    commit = strtrim(commit);
    filename = ['PerfComparison' date];
    if(~error)
        title(['Commit ' commit]);
        filename = ['PerfComparison-' commit];
    end
    print(filename,'-dpng');
    
    max(rel_mongoose_times)
    max(rel_metis_times)
end