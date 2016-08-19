function partition = mongoose_computeEdgeCut(G_safe,O,A)

%G_safe = (G+G')/2;

%[~,C] = graphconncomp(G_safe);
%G_safe = G_safe(C,C);

%G_safe = G_safe - diag(diag(G));

if nargin == 1
    partition = mongoose_computeEdgeSeparator(G_safe);
elseif nargin == 2
    partition = mongoose_computeEdgeSeparator(G_safe,O);
else
    partition = mongoose_computeEdgeSeparator(G_safe,O,A);
end