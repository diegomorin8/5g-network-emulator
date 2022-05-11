/**********************************************
* Copyright 2022 Nokia
* Licensed under the BSD 3-Clause Clear License
* SPDX-License-Identifier: BSD-3-Clause-Clear
**********************************************/

#include <pdcp_layer/release_handler.h>
#include <pdcp_layer/pdcp_config.h>

release_handler::release_handler(int _verbosity)
                :gauss_dist_gen(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
{
    verbosity = _verbosity; 
}


release_handler::~release_handler()
{
    quit(); 
}

void release_handler::quit()
{
}

void release_handler::init()
{
}

void release_handler::push(harq_pkt pkt)
{
    pkt.t_out += pkt.backhaul_d + gauss_dist(gauss_dist_gen)*pkt.backhaul_d_var;
    pkt_list.push_back(std::move(pkt));
}

float release_handler::release_one()
{
    return release(); 
}

int release_handler::get_size()
{
    int size = pkt_list.size();
    return size; 
}

float release_handler::release()
{
    int count = 0; 
    float bits = 0; 
    //std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    for(std::deque<harq_pkt>::iterator it=pkt_list.begin(); it!=pkt_list.end();)
    {
        if(it->t_out <= current_t)
        {
            bits += it->bits;
            if(verbosity > 0)
            {
                l_mean.add(current_t - it->current_t);
                ipl_mean.add(current_t - it->ip_t);
            }
            it = pkt_list.erase(it);
            count++; 
        }
        else it++; 
    }
   
    if(verbosity > 0) tp_mean.add(bits);
    return bits; 
}

void release_handler::step(float t)
{
    if(verbosity > 0)
    {
        tp_mean.step();
        l_mean.step(); 
        ipl_mean.step(); 
    }
    current_t = t; 
}


void release_handler::release_thread()
{
    while(is_awake)
    {
        release(); 
    }
    
}

float release_handler::get_latency(bool elapsed)
{
    if(verbosity > 0) 
    {
        if(!elapsed)return l_mean.get_total(); 
        else return l_mean.get(); 
    }
    return -1;
}


float release_handler::get_ip_latency(bool elapsed)
{
    if(verbosity > 0) 
    {
        if(!elapsed)return ipl_mean.get_total(); 
        else return ipl_mean.get(); 
    }
    return -1;
}


float release_handler::get_tp(bool elapsed)
{
    if(verbosity > 0)
    {
        if(!elapsed)return BIT2MBIT*tp_mean.get_total()*S2MS; 
        else return BIT2MBIT*tp_mean.get()*S2MS; 
    }
    return -1; 
}

