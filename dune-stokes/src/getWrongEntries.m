function y = getWrongEntries( felix, mirko )
size_felix = size( felix );
size_mirko = size( mirko );
if ( size_felix == size_mirko )
    for i = 1:size_felix(1)
        for j = 1:size_felix(2)
            felix_i_j = felix( i, j );
            mirko_i_j =  mirko( i, j );
            tmp = felix_i_j - mirko_i_j;
            if ( abs( tmp ) > 1e-10 )
                fprintf( 1, '%2d, %2d: is %4f but should be %4f\n', i - 1, j - 1, felix_i_j, mirko_i_j );
            end
        end
    end
end
y = 0;