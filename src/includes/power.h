/*
 * =======================================================================================
 *
 *      Filename:  power.h
 *
 *      Description:  Header File Power Module
 *                    Implements Intel RAPL Interface.
 *
 *      Version:   <VERSION>
 *      Released:  <DATE>
 *
 *      Author:  Jan Treibig (jt), jan.treibig@gmail.com
 *      Project:  likwid
 *
 *      Copyright (C) 2013 Jan Treibig 
 *
 *      This program is free software: you can redistribute it and/or modify it under
 *      the terms of the GNU General Public License as published by the Free Software
 *      Foundation, either version 3 of the License, or (at your option) any later
 *      version.
 *
 *      This program is distributed in the hope that it will be useful, but WITHOUT ANY
 *      WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *      PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along with
 *      this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =======================================================================================
 */

#ifndef POWER_H
#define POWER_H

#include <types.h>
#include <registers.h>
#include <bitUtil.h>
#include <msr.h>
#include <error.h>
#include <access.h>

const uint32_t power_regs[NUM_POWER_DOMAINS] = {MSR_PKG_ENERGY_STATUS,
                                MSR_PP0_ENERGY_STATUS,
                                MSR_PP1_ENERGY_STATUS,
                                MSR_DRAM_ENERGY_STATUS};

const char* power_names[NUM_POWER_DOMAINS] = {"PKG", "PP0", "PP1", "DRAM"};


double
power_printEnergy(PowerData* data)
{
    return  (double) ((data->after - data->before) * power_info.energyUnits[data->domain]);
}

int
power_start(PowerData* data, int cpuId, PowerType type)
{
    if (power_info.hasRAPL)
    {
        if ((power_info.supportedTypes & (1<<type)) != 0)
        {
            uint64_t result = 0;
            data->before = 0;
            CHECK_MSR_READ_ERROR(HPMread(cpuId, MSR_DEV, power_regs[type], &result))
            data->before = extractBitField(result,32,0);
            data->domain = type;
            return 0;
        }
        else
        {
            DEBUG_PRINT(DEBUGLEV_DEVELOP, RAPL domain %s not supported, power_names[type]);
            return -EFAULT;
        }
    }
    else
    {
        DEBUG_PLAIN_PRINT(DEBUGLEV_DEVELOP, No RAPL support);
        return -EIO;
    }
}

int
power_stop(PowerData* data, int cpuId, PowerType type)
{
    if (power_info.hasRAPL)
    {
        if ((power_info.supportedTypes & (1<<type)) != 0)
        {
            uint64_t result = 0;
            data->after = 0;
            CHECK_MSR_READ_ERROR(HPMread(cpuId, MSR_DEV, power_regs[type], &result))
            data->after = extractBitField(result,32,0);
            data->domain = type;
            return 0;
        }
        else
        {
            DEBUG_PRINT(DEBUGLEV_DEVELOP, RAPL domain %s not supported, power_names[type]);
            return -EFAULT;
        }
    }
    else
    {
        DEBUG_PLAIN_PRINT(DEBUGLEV_DEVELOP, No RAPL support);
        return -EIO;
    }
}

int
power_read(int cpuId, uint64_t reg, uint32_t *data)
{
    int i;
    PowerType type = -1;

    if (power_info.hasRAPL)
    {
        for (i = 0; i < 4; i++)
        {
            if (reg == power_regs[i])
            {
                type = i;
                break;
            }
        }
        if ((power_info.supportedTypes & (1<<type)) != 0)
        {
            uint64_t result = 0;
            *data = 0;
            CHECK_MSR_READ_ERROR(HPMread(cpuId, MSR_DEV, reg, &result))
            *data = extractBitField(result,32,0);
            return 0;
        }
        else
        {
            DEBUG_PRINT(DEBUGLEV_DEVELOP, RAPL domain %s not supported, power_names[type]);
            return -EFAULT;
        }
    }
    else
    {
        DEBUG_PLAIN_PRINT(DEBUGLEV_DEVELOP, No RAPL support);
        return -EIO;
    }
}

int
power_tread(int socket_fd, int cpuId, uint64_t reg, uint32_t *data)
{
    int i;
    PowerType type;
    if (power_info.hasRAPL)
    {
        for (i = 0; i < 4; i++)
        {
            if (reg == power_regs[i])
            {
                type = i;
                break;
            }
        }
        if ((power_info.supportedTypes & (1<<type)) != 0)
        {
            uint64_t result = 0;
            *data = 0;
            CHECK_MSR_READ_ERROR(HPMread(cpuId, MSR_DEV, reg, &result))
            *data = extractBitField(result,32,0);
            return 0;
        }
        else
        {
            DEBUG_PRINT(DEBUGLEV_DEVELOP, RAPL domain %s not supported, power_names[type]);
            return -EFAULT;
        }
    }
    else
    {
        DEBUG_PLAIN_PRINT(DEBUGLEV_DEVELOP, No RAPL support);
        return -EIO;
    }
}

double
power_getEnergyUnit(int domain)
{
    return power_info.energyUnits[domain];
}

#endif /*POWER_H*/
