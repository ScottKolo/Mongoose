function partition = mongoose_computeEdgeCut(G,O,A)

G_safe = mongoose_sanitizeMatrix(G);

if nargin == 1
    partition = mongoose_computeEdgeSeparator(G_safe);
elseif nargin == 2
    partition = mongoose_computeEdgeSeparator(G_safe,O);
else
    partition = mongoose_computeEdgeSeparator(G_safe,O,A);
end