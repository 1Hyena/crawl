/*
 *  File:       message.cc
 *  Summary:    Functions used to print messages.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author: zelgadis $ on $Date: 2007-09-16 00:33:50 +0100 (Sun, 16 Sep 2007) $
 *
 *  Modified for Hexcrawl by Martin Bays, 2007
 *
 *  Change History (most recent first):
 *
 *               <2>     5/08/99        JDJ             mpr takes a const char* instead of a char array.
 *               <1>     -/--/--        LRH             Created
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <streambuf>
#include <iostream>

#include "enum.h"
#include "mpr.h"

struct message_item {
    msg_channel_type    channel;        // message channel
    int                 param;          // param for channel (god, enchantment)
    std::string         text;           // text of message
};


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: ability - acr - command - direct - effects - item_use -
 *              misc - player - spell - spl-book - spells1 - spells2 -
 *              spells3
 * *********************************************************************** */
void mesclr( bool force = false );


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - bang - beam - decks - fight - files - it_use3 -
 *              item_use - items - message - misc - ouch - player -
 *              religion - spell - spells - spells2 - spells3
 * *********************************************************************** */
void more(void);


class formatted_string;

void formatted_mpr(const formatted_string& fs,
                   msg_channel_type channel = MSGCH_PLAIN, int param = 0);
                   
void formatted_message_history(const std::string &st,
                               msg_channel_type channel = MSGCH_PLAIN,
                               int param = 0);

// mpr() an arbitrarily long list of strings
void mpr_comma_separated_list(const std::string prefix,
                              const std::vector<std::string> list,
                              const std::string &andc = " and ",
                              const std::string &comma = ", ",
                              const msg_channel_type channel = MSGCH_PLAIN,
                              const int param = 0);
                               
class no_messages
{
public:
    no_messages();
    ~no_messages();
private:
    bool msuppressed;
};

bool any_messages();
void replay_messages();

void set_colour(char set_message_colour);


// last updated 13oct2003 {dlb}
/* ***********************************************************************
 * called from: chardump
 * *********************************************************************** */
std::string get_last_messages(int mcount);

int channel_to_colour( msg_channel_type channel, int param = 0 );

namespace msg
{
    extern std::ostream stream;
    std::ostream& streams(msg_channel_type chan = MSGCH_PLAIN);

    struct setparam
    {
        setparam(int param);
        int m_param;
    };

    struct mute
    {
        mute(bool value = true);
        bool m_value;
    };

    class mpr_stream_buf : public std::streambuf
    {
    public:
        mpr_stream_buf(msg_channel_type chan);
        void set_param(int p);
        void set_muted(bool m);
    protected:
        int overflow(int c);
    private:
        static const int INTERNAL_LENGTH = 500;
        char internal_buf[500]; // if your terminal is wider than this, too bad
        int internal_count;
        int param;
        bool muted;
        msg_channel_type channel;
    };

    void initialise_mpr_streams();
    void deinitalise_mpr_streams();
}

std::ostream& operator<<(std::ostream& os, const msg::setparam& sp);


#endif
