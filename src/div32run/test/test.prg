/*
 * TEST.PRG for div32run
 *
 * Run this program in original DIV2 to generate testdata.h
 */

/* FUNCTIONS WE MAY TEST:
    get_angle()         Done!
    get_dist()          Done!
    get_distx()         Done!
    get_disty()         Done!
    advance()           Done!
    fget_angle()
    fget_dist()
    near_angle()
    xadvance()
    abs()               Done!
    acos()
    asin()
    atan()
    atan2()
    cos()
    pow()
    sin()
    sqrt()              Done!
    tan()

    find_color()

    char()
    lower()
    strcat()
    strchr()
    strcmp()
    strcpy()
    strdel()
    strlen()
    strset()
    strstr()
    upper()

    calculate()
    itoa()

    get_real_point()

    load()
    save()
    fclose()
    filelength()
    flush()
    fopen()
    fread()
    fseek()
    ftell()
    fwrite()

    compress_file()
    uncompress_file()
    decode_file()
    encode()
    encode_file()
*/

program test;
const
    CR = 0x0D;

private
    int angles[] = 0, 1, pi, -pi, pi/2, -pi/2, -34939, 25413, 128373, 141729, 191278, 239747, 202220, 266022, 296266, 277990;
    int distances[] = 0, 1, 2, 8, 22, 64, 143, 339, 976, 3071, 8233, -450, -6355;
    int coord_x[] = -7, 21, -47, 154, 345, 1225, -2523, 8665, -26886, -51602;
    int coord_y[] = 7, -20, 67, 132, -332, -1222, -2672, -6360, 22321, -68255;
    int data_abs[] = 0, -6, 17, 56, -132, -370, -1307, -2337, 7057, 19004, -70437;
    f;
    i,j,k,l;
    proc;
    int length;
    string s;
begin
    unit_size = 1;
    proc = dummy();

    f = fopen("testdata.h", "w");

    fputs("int data_get_angle[]={"+CR, f);
    length = 0;
    for(i=0; i<sizeof(coord_x); i+=2)
        for(j=0; j<sizeof(coord_y); j+=2)
            x = coord_x[i];
            y = coord_y[j];
            proc.x = coord_x[i+1];
            proc.y = coord_x[j+1];
            fputs(itoa(x)+","+itoa(y)+","+itoa(proc.x)+","+itoa(proc.y)+","+itoa(get_angle(proc))+","+CR, f);
            length += 5;
        end
    end
    fputs("};"+CR, f);
    fputs("const int data_get_angle_length=" + itoa(length)+";"+CR, f);

    fputs("int data_get_dist[]={"+CR, f);
    length = 0;
    for(i=0; i<sizeof(coord_x); i+=2)
        for(j=0; j<sizeof(coord_y); j+=2)
            x = coord_x[i];
            y = coord_y[j];
            proc.x = coord_x[i+1];
            proc.y = coord_x[j+1];
            fputs(itoa(x)+","+itoa(y)+","+itoa(proc.x)+","+itoa(proc.y)+","+itoa(get_dist(proc))+","+CR, f);
            length += 5;
        end
    end
    proc.x = x;
    proc.y = y;
    fputs(itoa(x)+","+itoa(y)+","+itoa(proc.x)+","+itoa(proc.y)+","+itoa(get_dist(proc))+","+CR, f);
    length++;
    fputs("};"+CR, f);
    fputs("const int data_get_dist_length=" + itoa(length)+";"+CR, f);

    fputs("int data_get_distx[]={"+CR, f);
    length = 0;
    for(i=0; i<sizeof(angles); i++)
        for(j=0; j<sizeof(distances); j++)
            fputs(itoa(angles[i])+","+itoa(distances[j])+","+itoa(get_distx(angles[i], distances[j]))+","+CR, f);
            length += 3;
        end
    end
    fputs("};"+CR, f);
    fputs("const int data_get_distx_length=" + itoa(length)+";"+CR, f);

    fputs("int data_get_disty[]={"+CR, f);
    length = 0;
    for(i=0; i<sizeof(angles); i++)
        for(j=0; j<sizeof(distances); j++)
            fputs(itoa(angles[i])+","+itoa(distances[j])+","+itoa(get_disty(angles[i], distances[j]))+","+CR, f);
            length += 3;
        end
    end
    fputs("};"+CR, f);
    fputs("const int data_get_disty_length=" + itoa(length)+";"+CR, f);

    fputs("int data_advance[]={"+CR, f);
    length = 0;
    // vamo a calmarno...
    for(i=0; i<sizeof(angles); i+=2)
        for(j=0; j<sizeof(distances); j+=2)
            for(k=0; k<sizeof(coord_x); k+=2)
                for(l=0; l<sizeof(coord_y); l+=2)
                    x = coord_x[k];
                    y = coord_y[l];
                    angle = angles[i];
                    advance(distances[j]);
                    fputs(itoa(angles[i])+","
                        +itoa(distances[j])+","
                        +itoa(coord_x[k])+","
                        +itoa(coord_y[l])+","
                        +itoa(x)+","
                        +itoa(y)+","+CR, f);
                    length += 6;
                end
            end
        end
    end
    fputs("};"+CR, f);
    fputs("const int data_advance_length=" + itoa(length)+";"+CR, f);

    fputs("int data_abs[]={"+CR, f);
    length = 0;
    for(i=0; i<sizeof(data_abs); i++)
        fputs(itoa(data_abs[i])+","+itoa(abs(data_abs[i]))+","+CR, f);
        length += 2;
    end
    fputs("};"+CR, f);
    fputs("const int data_abs_length=" + itoa(length)+";"+CR, f);

    fputs("int data_sqrt[]={"+CR, f);
    length = 0;
    for(i=0; i<sizeof(angles); i++)
        fputs(itoa(angles[i])+","+itoa(sqrt(angles[i]))+","+CR, f);
        length += 2;
    end
    fputs("};"+CR, f);
    fputs("const int data_sqrt_length=" + itoa(length)+";"+CR, f);

    fclose(f);
end

function fputs(string str, fp)
begin
    fwrite(&str, strlen(str), fp);
end

process dummy()
begin
end