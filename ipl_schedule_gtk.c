#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>


typedef struct
{
    int no;
    char date[20];
    char time[10];
    char team1[50];
    char team2[50];
    char venue[100];
    int day;
    int month;
} ScheduleEntry;

typedef struct
{
    int no;
    char date[20];
    char time[10];
    char team1[50];
    char team2[50];
    char venue[100];
    int runs1;
    float overs1;
    int runs2;
    float overs2;
    char winner[50];
} MatchResult;

typedef struct
{
    int pos;
    char team[50];
    int mp;
    int won;
    int lost;
    int pts;
    float nrr;
} PointsEntry;


#define MAX_TEAMS 10
#define MAX_LINE_LEN 512
#define MAX_SCHEDULE_ENTRIES 100
#define MAX_RESULTS_ENTRIES 100

ScheduleEntry schedule[MAX_SCHEDULE_ENTRIES];
int schedule_count = 0;
MatchResult results[MAX_RESULTS_ENTRIES];
int results_count = 0;
PointsEntry points[MAX_TEAMS];
int points_count = 0;

// Color definitions for renderer background binding
const char *odd_row_bg_color_str = "#1A4E95";  // Darker Blue
const char *even_row_bg_color_str = "#2B66B2"; // Lighter Blue
// Color definition for text foreground
const char *text_fg_color_str = "#FFFFFF";

// Helper for trimming whitespace
char *trimwhitespace(char *str)
{
  char *end;
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0) return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}



int read_schedule(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) { perror("Error opening schedule file"); return 0; }

    char header[MAX_LINE_LEN];
    fgets(header, sizeof(header), file); // Skip the header

    schedule_count = 0;
    while (schedule_count < MAX_SCHEDULE_ENTRIES &&
           fscanf(file, " %49[^,],%49[^,],%99[^,],%d,%d,%9[^\n]\n",
                  schedule[schedule_count].team1,
                  schedule[schedule_count].team2,
                  schedule[schedule_count].venue,
                  &schedule[schedule_count].day,
                  &schedule[schedule_count].month,
                  schedule[schedule_count].time) == 6)
    {
        sprintf(schedule[schedule_count].date, "%02d/%02d",
                schedule[schedule_count].day,
                schedule[schedule_count].month);
        schedule[schedule_count].no = schedule_count + 1;
        schedule_count++;
    }

    fclose(file);
    printf("Read %d schedule entries from %s\n", schedule_count, filename);
    return schedule_count > 0;
}


int read_match_results(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening match results file");
        return 0;
    }

    char header_buffer[MAX_LINE_LEN];
    results_count = 0;

    // Skip the header line
    if (fgets(header_buffer, sizeof(header_buffer), file) == NULL) {
        fclose(file);
        return 0;
    }

    while (results_count < MAX_RESULTS_ENTRIES) {
        MatchResult *r = &results[results_count];

        int read = fscanf(file, "%d,%19[^,],%9[^,],%49[^,],%49[^,],%49[^,],%d,%f,%d,%f,%49[^\n]\n",
            &r->no,
            r->date,
            r->time,
            r->team1,
            r->team2,
            r->venue,
            &r->runs1,
            &r->overs1,
            &r->runs2,
            &r->overs2,
            r->winner
        );

        if (read == EOF) break;  // End of file

        if (read != 11) {
            fprintf(stderr, "Warning: Malformed line in %s (read fields: %d)\n", filename, read);
            continue;
        }

        results_count++;
    }

    fclose(file);
    printf("Read %d match result entries from %s\n", results_count, filename);
    return results_count > 0;
}

int read_points_table(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening points table file");
        return 0;
    }

    char header_buffer[MAX_LINE_LEN];
    points_count = 0;

    // Skip the header line
    if (fgets(header_buffer, sizeof(header_buffer), file) == NULL) {
        fclose(file);
        return 0;
    }

    while (points_count < MAX_TEAMS) {
        PointsEntry *p = &points[points_count];

        int read = fscanf(file, "%d,%49[^,],%d,%d,%d,%d,%f\n",
            &p->pos,
            p->team,
            &p->mp,
            &p->won,
            &p->lost,
            &p->pts,
            &p->nrr
        );

        if (read == EOF) break;  // End of file

        if (read != 7) {
            fprintf(stderr, "Warning: Malformed line in %s (read fields: %d)\n", filename, read);
            continue;
        }

        points_count++;
    }

    fclose(file);
    printf("Read %d points table entries from %s\n", points_count, filename);
    return points_count > 0;
}




enum { SCHED_COL_NO, SCHED_COL_DATE, SCHED_COL_TIME, SCHED_COL_TEAM1, SCHED_COL_VS, SCHED_COL_TEAM2, SCHED_COL_VENUE, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR, NUM_SCHED_COLS };
enum { RESULTS_COL_NO, RESULTS_COL_DATE, RESULTS_COL_TIME, RESULTS_COL_TEAM1, RESULTS_COL_SCORE1, RESULTS_COL_VS, RESULTS_COL_TEAM2, RESULTS_COL_SCORE2, RESULTS_COL_WINNER, RESULTS_COL_VENUE, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR, NUM_RESULTS_COLS };
enum { POINTS_COL_POS, POINTS_COL_TEAM, POINTS_COL_MP, POINTS_COL_WON, POINTS_COL_LOST, POINTS_COL_PTS, POINTS_COL_NRR, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR, NUM_POINTS_COLS };


GtkTreeViewColumn *create_text_column(const char *title, int col_id, gfloat xalign, int bg_col_id, int fg_col_id) // Added fg_col_id
{
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_NONE, "yalign", 0.5, "xalign", xalign, NULL);

    // Create column BEFORE adding attributes
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", col_id, NULL);

    gtk_tree_view_column_add_attribute(column, renderer, "cell-background", bg_col_id);

    gtk_tree_view_column_add_attribute(column, renderer, "foreground", fg_col_id);


    gtk_tree_view_column_set_sort_column_id(column, col_id);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_alignment(column, xalign);

    if (strcmp(title, "No.") == 0 || strcmp(title, "Pos") == 0 || strcmp(title, "") == 0)
        gtk_tree_view_column_set_min_width(column, 40);
    else if (strcmp(title, "MP") == 0 || strcmp(title, "W") == 0 || strcmp(title, "L") == 0 || strcmp(title, "Pts") == 0)
         gtk_tree_view_column_set_min_width(column, 35);
    else if (strcmp(title, "NRR") == 0)
        gtk_tree_view_column_set_min_width(column, 65);
    else if (strcmp(title, "Date") == 0 || strcmp(title, "Time") == 0)
        gtk_tree_view_column_set_min_width(column, 75);
     else if (strcmp(title, "Score") == 0)
         gtk_tree_view_column_set_min_width(column, 70);

    return column;
}


GtkWidget *create_schedule_page()
{
    GtkWidget *sw = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   
    GtkListStore *ls = gtk_list_store_new(NUM_SCHED_COLS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter;
    for (int i = 0; i < schedule_count; i++) {
        const char *bg_color = (i % 2 == 0) ? even_row_bg_color_str : odd_row_bg_color_str;
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter,
                           SCHED_COL_NO, schedule[i].no,
                           SCHED_COL_DATE, schedule[i].date,
                           SCHED_COL_TIME, schedule[i].time,
                           SCHED_COL_TEAM1, schedule[i].team1,
                           SCHED_COL_VS, "vs",
                           SCHED_COL_TEAM2, schedule[i].team2,
                           SCHED_COL_VENUE, schedule[i].venue,
                           SCHED_COL_BG_COLOR, bg_color,
                           SCHED_COL_FG_COLOR, text_fg_color_str, // *** Set foreground color string ***
                           -1);
    }
    GtkWidget *tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
    gtk_widget_set_name(tv, "ipl-treeview");
    g_object_unref(ls);

    GtkTreeViewColumn *col;
    // *** MODIFIED calls to create_text_column to include FG color column index ***
    col = create_text_column("No.", SCHED_COL_NO, 0.5, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Date", SCHED_COL_DATE, 0.5, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Time", SCHED_COL_TIME, 0.5, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Home", SCHED_COL_TEAM1, 0.0, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("", SCHED_COL_VS, 0.5, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Away", SCHED_COL_TEAM2, 0.0, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Venue", SCHED_COL_VENUE, 0.0, SCHED_COL_BG_COLOR, SCHED_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), tv);
    gtk_widget_set_vexpand(sw, TRUE);
    gtk_widget_set_hexpand(sw, TRUE);
    return sw;
}

GtkWidget *create_results_page()
{
    GtkWidget *sw = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    // *** MODIFIED ListStore ***
    GtkListStore *ls = gtk_list_store_new(NUM_RESULTS_COLS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter; char s1[50], s2[50];
    for (int i = 0; i < results_count; i++) {
        const char *bg_color = (i % 2 == 0) ? even_row_bg_color_str : odd_row_bg_color_str;
        snprintf(s1, sizeof(s1), "%d (%.1f)", results[i].runs1, results[i].overs1);
        snprintf(s2, sizeof(s2), "%d (%.1f)", results[i].runs2, results[i].overs2);
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter,
                           RESULTS_COL_NO, results[i].no,
                           RESULTS_COL_DATE, results[i].date,
                           RESULTS_COL_TIME, results[i].time,
                           RESULTS_COL_TEAM1, results[i].team1,
                           RESULTS_COL_SCORE1, s1,
                           RESULTS_COL_VS, "vs",
                           RESULTS_COL_TEAM2, results[i].team2,
                           RESULTS_COL_SCORE2, s2,
                           RESULTS_COL_WINNER, results[i].winner,
                           RESULTS_COL_VENUE, results[i].venue,
                           RESULTS_COL_BG_COLOR, bg_color,
                           RESULTS_COL_FG_COLOR, text_fg_color_str, // *** Set foreground color string ***
                           -1);
    }
    GtkWidget *tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
    gtk_widget_set_name(tv, "ipl-treeview");
    g_object_unref(ls);

    GtkTreeViewColumn *col;
    col = create_text_column("No.", RESULTS_COL_NO, 0.5, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Date", RESULTS_COL_DATE, 0.5, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Time", RESULTS_COL_TIME, 0.5, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Home", RESULTS_COL_TEAM1, 0.0, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Score", RESULTS_COL_SCORE1, 1.0, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("", RESULTS_COL_VS, 0.5, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Away", RESULTS_COL_TEAM2, 0.0, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Score", RESULTS_COL_SCORE2, 1.0, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Result", RESULTS_COL_WINNER, 0.0, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Venue", RESULTS_COL_VENUE, 0.0, RESULTS_COL_BG_COLOR, RESULTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), tv);
    gtk_widget_set_vexpand(sw, TRUE);
    gtk_widget_set_hexpand(sw, TRUE);
    return sw;
}

GtkWidget *create_points_page()
{
    GtkWidget *sw = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkListStore *ls = gtk_list_store_new(NUM_POINTS_COLS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter; char nrr_str[20];
    for (int i = 0; i < points_count; i++) {
        const char *bg_color = (i % 2 == 0) ? even_row_bg_color_str : odd_row_bg_color_str;
        snprintf(nrr_str, sizeof(nrr_str), "%.3f", points[i].nrr);
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter,
                           POINTS_COL_POS, points[i].pos,
                           POINTS_COL_TEAM, points[i].team,
                           POINTS_COL_MP, points[i].mp,
                           POINTS_COL_WON, points[i].won,
                           POINTS_COL_LOST, points[i].lost,
                           POINTS_COL_PTS, points[i].pts,
                           POINTS_COL_NRR, nrr_str,
                           POINTS_COL_BG_COLOR, bg_color,
                           POINTS_COL_FG_COLOR, text_fg_color_str, 
                           -1);
    }
    GtkWidget *tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
    gtk_widget_set_name(tv, "ipl-treeview");
    g_object_unref(ls);

    GtkTreeViewColumn *col;
    
    col = create_text_column("Pos", POINTS_COL_POS, 0.5, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Team", POINTS_COL_TEAM, 0.0, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, TRUE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("MP", POINTS_COL_MP, 0.5, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("W", POINTS_COL_WON, 0.5, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("L", POINTS_COL_LOST, 0.5, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("Pts", POINTS_COL_PTS, 0.5, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
    col = create_text_column("NRR", POINTS_COL_NRR, 1.0, POINTS_COL_BG_COLOR, POINTS_COL_FG_COLOR); gtk_tree_view_column_set_expand(col, FALSE); gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), tv);
    gtk_widget_set_vexpand(sw, TRUE);
    gtk_widget_set_hexpand(sw, TRUE);
    return sw;
}


// --- Simplified CSS (v14 - Test Basic Applicability) ---
static void load_custom_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();

    const char *font_family     = "\"Segoe UI\", Ubuntu, Cantarell, sans-serif";
    const char *font_size       = "9pt";
    const char *font_size_large = "11pt";
    const char *main_heading_bg = " #183787";
    const char *tab_inactive_bg = "#E0E8F0";
    const char *tab_inactive_fg = "#305070";
    const char *tab_active_bg   = "#FFFFFF";
    const char *tab_active_fg   = "#1A4E95";
    const char *tab_area_bg     = "#C0D0E0";
    const char *table_header_bg = "#D91E2A";
    const char *table_header_fg = "#FFFFFF";
    const char *table_header_border = "#B01520";

    char css_buffer[4096];
    snprintf(css_buffer, sizeof(css_buffer),
        "/* --- Heading Styling --- */\n"
        ".ipl-heading { background-color: %s; color: white; padding: 5px 10px; border: none; }\n"
        ".ipl-heading label { font-family: %s; font-size: %s; font-weight: bold; }\n"
        "\n"
        "/* --- Notebook Tabs --- */\n"
        ".ipl-notebook tab { padding: 6px 12px; font-family: %s; font-size: %s; background-color: %s; color: %s; border-radius: 4px 4px 0 0; border: 1px solid transparent; border-bottom: none; margin-right: 2px; }\n"
        ".ipl-notebook tab:hover { background-color: #EFF4FA; }\n"
        ".ipl-notebook tab:checked { font-weight: bold; background-color: %s; color: %s; border: 1px solid #A0B0C0; border-bottom: 1px solid %s; }\n"
        ".ipl-notebook > header.top { background-color: %s; border-bottom: 1px solid #A0B0C0; padding: 0; }\n"
        "\n"
        "/* --- TreeView Header --- */\n"
        ".ipl-notebook treeview header button { background-image: none; background-color: %s; color: %s; font-family: %s; font-size: %s; font-weight: bold; border: none; padding: 8px; border-right: 1px solid %s; }\n"
        ".ipl-notebook treeview header button:last-child { border-right: none; }\n"
        ".ipl-notebook treeview header { background-color: %s; border-bottom: 2px solid %s; }\n"
        "\n"
        "/* --- !!! SIMPLIFIED TREEVIEW TEST !!! --- */\n"
        "#ipl-treeview { background-color: lightgrey !important; } /* Base background */ \n"
        "\n"
        "#ipl-treeview cell { \n"
        "   background-color: lime !important; /* Test BG */ \n"
        "   border: none !important; \n" // Ensure no borders interfere
        "   padding: 7px 6px; \n"
        "}\n"
        "\n"
        "#ipl-treeview cell label { \n"
        "   color: white !important; /* Test Text Color */ \n"
        "}\n"
        "\n"
        ".ipl-notebook treeview {\n" // Still disable internal separators
        "   -GtkTreeView-horizontal-separator: 0;\n"
        "   -GtkTreeView-vertical-separator: 0;\n"
        "}\n"

       ,
        /* Arguments */
        main_heading_bg, font_family, font_size_large, // Heading
        font_family, font_size, tab_inactive_bg, tab_inactive_fg, // Tabs
        tab_active_bg,  tab_active_fg, tab_active_bg, tab_area_bg, // Tabs
        table_header_bg, table_header_fg, font_family, font_size, table_header_border, // Tree Header
        table_header_bg, table_header_border // Tree Header
        // No other args needed for simplified CSS
    );

    // Load CSS
    gtk_css_provider_load_from_string(provider, css_buffer);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
    g_print("Custom IPL CSS Loaded (v14 - SIMPLIFIED TEST).\n");
}


// --- Activate Function (Keep as before) ---
static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window, *vbox, *hbox_heading, *heading_label, *notebook, *logo_image;
    GdkPixbuf *pixbuf = NULL;
    GError *error = NULL;
    const char *logo_filename = "IPL-logo.jpg"; // Ensure this path is correct!

    // --- Define desired size for the logo ---
    int requested_logo_width = 120;  // Set your desired width here
    int requested_logo_height = 100; // Set to -1 to let GTK calculate height based on width & aspect ratio
                                    // Or set a specific height (e.g., 90), but this might distort the image

    // --- Load Data ---
    if (schedule_count == 0 && !read_schedule("ipl_schedule.csv")) { fprintf(stderr, "Failed to read schedule data for UI.\n"); }
    if (results_count == 0 && !read_match_results("matches.csv")) { fprintf(stderr, "Failed to read match results data for UI.\n"); }
    if (points_count == 0 && !read_points_table("points_table.csv")) { fprintf(stderr, "Failed to read points table data for UI.\n"); }


    // --- Load CSS (might still be used for other elements) ---
    load_custom_css();

    // --- Create Window and Main Layout ---
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "IPL Data Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 700);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // --- Create Heading Area ---
    hbox_heading = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15); // Spacing
    gtk_widget_add_css_class(hbox_heading, "ipl-heading");
    gtk_widget_set_halign(hbox_heading, GTK_ALIGN_FILL);
    gtk_widget_set_valign(hbox_heading, GTK_ALIGN_CENTER); // Center vertically

    // --- Load Logo Pixbuf (for checking and creating image) ---
    pixbuf = gdk_pixbuf_new_from_file(logo_filename, &error);

    if (!pixbuf) {
        // --- Error Handling ---
        g_printerr("ERROR: Failed to load logo image '%s'.\n", logo_filename);
        g_printerr("Reason: %s\n", error ? error->message : "Unknown (gdk_pixbuf_new_from_file failed)");
        g_printerr("Check file path/permissions/format. Current working directory might be wrong.\n");
        if (error) g_error_free(error);

        // Create a fallback label
        logo_image = gtk_label_new("[Logo Missing]");
        gtk_widget_set_valign(logo_image, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(logo_image, GTK_ALIGN_START);
    } else {
        // --- Create GtkImage from the *original* pixbuf ---
        logo_image = gtk_image_new_from_pixbuf(pixbuf);

        // --- Control Size via gtk_widget_set_size_request() ---
        // This directly requests a size for the GtkImage widget.
        // Using -1 for height preserves the aspect ratio based on the requested width.
        gtk_widget_set_size_request(logo_image, requested_logo_width, requested_logo_height);
        // Note: While this requests a size, the final size might still be affected
        // by container constraints and other layout properties.
        
        // --- Standard Widget Setup ---
        gtk_widget_set_tooltip_text(logo_image, "IPL Logo");
        gtk_widget_set_halign(logo_image, GTK_ALIGN_START);
        gtk_widget_set_valign(logo_image, GTK_ALIGN_FILL);
        gtk_widget_set_hexpand(logo_image, FALSE); // Don't expand horizontally
        gtk_widget_set_vexpand(logo_image, FALSE); // Don't expand vertically

        // --- Free the pixbuf ---
        g_object_unref(pixbuf);
    }

    // Add the logo widget (either image or label) to the heading box
    gtk_box_append(GTK_BOX(hbox_heading), logo_image);

    // --- Create Heading Label ---
    heading_label = gtk_label_new("IPL SEASON SCHEDULE");
    gtk_widget_add_css_class(heading_label, "ipl-title"); // Keep CSS class for label styling
    gtk_widget_set_halign(heading_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(heading_label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(heading_label, TRUE); // Label expands
    gtk_widget_set_vexpand(heading_label, FALSE); 
    gtk_box_append(GTK_BOX(hbox_heading), heading_label);

    // Add heading HBox to main VBox
    gtk_box_append(GTK_BOX(vbox), hbox_heading);

    // --- Create and Add Notebook ---
    notebook = gtk_notebook_new();
    gtk_widget_add_css_class(notebook, "ipl-notebook");
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);

    // --- Create Pages ---
    if (schedule_count > 0) { gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_schedule_page(), gtk_label_new("Schedule")); }
    else { gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Could not load schedule.csv"), gtk_label_new("Schedule")); }
    if (results_count > 0) { gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_results_page(), gtk_label_new("Match Results")); }
    else { gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Could not load matches.csv"), gtk_label_new("Match Results")); }
    if (points_count > 0) { gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_points_page(), gtk_label_new("Points Table")); }
    else { gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Could not load points_table.csv"), gtk_label_new("Points Table")); }


    // Add notebook to main VBox
    gtk_box_append(GTK_BOX(vbox), notebook);

    // Show the window
    gtk_window_present(GTK_WINDOW(window));
}

// --- Main Function ---
int main(int argc, char *argv[])
{
    read_schedule("ipl_schedule.csv");
    read_match_results("matches.csv");
    read_points_table("points_table.csv");
    GtkApplication *app = gtk_application_new("org.example.ipldisplay.gtk.v13", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}