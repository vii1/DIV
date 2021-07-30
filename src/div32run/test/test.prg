/*
 * TEST.PRG for div32run
 *
 * Run this program in original DIV2 to generate testdata.h
 */

/* FUNCTIONS WE MAY TEST:
    get_angle()			Done!
    get_dist()
    get_distx()
    get_disty()
    advance()
    fget_angle()
    fget_dist()
    near_angle()
    xadvance()
    abs()
    acos()
    asin()
    atan()
    atan2()
    cos()
    pow()
    sin()
    sqrt()
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
    int distances[] = 1, 2, 8, 22, 64, 143, 339, 976, 3071, 8233, -450, -6355;
    int coord_x[] = -7, 21, -47, 154, 345, 1225, -2523, 8665, -26886, -51602;
    int coord_y[] = 7, -20, 67, 132, -332, -1222, -2672, -6360, 22321, -68255;
    int data_abs[] = 0, -6, 17, 56, -132, -370, -1307, -2337, 7057, 19004, -70437;
    f;
    i,j;
    proc;
    string s;
begin
    unit_size = 1;
    proc = dummy();

    f = fopen("testdata.h", "w");

    fputs("int data_get_angle[]={"+CR, f);
    for(i=0; i<sizeof(coord_x); i+=2)
        for(j=0; j<sizeof(coord_y); j+=2)
            x = coord_x[i];
            y = coord_y[j];
            proc.x = coord_x[i+1];
            proc.y = coord_x[j+1];
            fputs(itoa(x)+","+itoa(y)+","+itoa(proc.x)+","+itoa(proc.y)+","+itoa(get_angle(proc))+","+CR, f);
        end
    end
    fputs("};"+CR, f);

    fputs("int data_get_dist[]={"+CR, f);
    for(i=0; i<sizeof(coord_x); i+=2)
        for(j=0; j<sizeof(coord_y); j+=2)
            x = coord_x[i];
            y = coord_y[j];
            proc.x = coord_x[i+1];
            proc.y = coord_x[j+1];
            fputs(itoa(x)+","+itoa(y)+","+itoa(proc.x)+","+itoa(proc.y)+","+itoa(get_dist(proc))+","+CR, f);
        end
    end
	proc.x = x;
	proc.y = y;
	fputs(itoa(x)+","+itoa(y)+","+itoa(proc.x)+","+itoa(proc.y)+","+itoa(get_dist(proc))+","+CR, f);
    fputs("};"+CR, f);

    fputs("int data_abs[]={"+CR, f);
    for(i=0; i<sizeof(data_abs); i++)
		fputs(itoa(data_abs[i])+","+itoa(abs(data_abs[i]))+","+CR, f);
    end
    fputs("};"+CR, f);

    fclose(f);
end

function fputs(string str, fp)
begin
    fwrite(&str, strlen(str), fp);
end

process dummy()
begin
end