-- Put script in the client/scripts/ directory of your proxmark installation location
-- Call script from proxmark client
-- TODO clean up this mess

local reader = require('read14a')
local cmds = require('commands')
local utils = require('utils')
-- SHA1 is not included in lua, make sure you put sha1.lua in a location where it can be called
-- client/lualibs/ is a good default folder to place it
local sha1 = require('sha1')

local seedFile = 'mifareTagSeeds.txt'

--[[
    Try to be as random as possible.
    os.time() only changes once a second, os.clock depends on runtime.
    Use both in this case.

    DO NOT USE THIS UNLESS ABSOLUTELY NECESSARY.
    Lua's random function is not completely random.
    Use a different program instead to generate the seed.
    Then write the seed to the text file and "re"write it to the tag using this script.
]]
math.randomseed(os.time()*(os.clock()*1000000))

function tprint(tbl, indent)
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    else
      print(formatting .. v)
    end
  end
end

function execCommand(cmd)
    print(cmd)
    core.console(cmd)
end

function validateNewUid(uid)
    local file = io.open(seedFile, 'r')
    if file == nil then
        return true
    else
        for line in file:lines() do
            for k, v in string.gmatch(line, "(%w+) (%w+)") do
                if k == uid then
                    seed = v
                    return false
                end
            end
        end
        file:close()
        return true
    end
end

function generateSeed()
    -- Seed will be 8 bytes long
    seed = ''
    for i = 1, 16, 1 do
        local hexVal = math.random(0, 15)
        seed = seed .. string.format("%x", hexVal)
    end
    return true
end

function writeTagSeed(uid, seed)
    local f = io.open(seedFile, 'a')
    f:write(uid .. ' ' .. seed .. '\r\n')
end

function deleteTagSeed(uid)
    local file = io.open(seedFile, 'r')

    local fileContents = ''
    for line in file:lines() do
        for k, v in string.gmatch(line, "(%w+) (%w+)") do
            if k ~= uid then
                fileContents = fileContents .. k .. ' ' .. v .. '\r\n'
            end
        end
    end
    file:close()

    local file = io.open(seedFile, 'w')
    file:write(fileContents)
    file:close()
end

function writeToTag(mode)
    -- Mode:
    --  0 = reset to default keys of 12*F
    --  1 = set keys based on seed
    -- Generate keys
    print('Writing data to tag')
    for sectorNum = 0, 15 do
        -- Generate key and data for sector
        sectorHash = sha1.hex(seed .. sectorNum)
        if mode == 1 then
            sectorKey = string.sub(sectorHash, 1, 12)
            sectorData = string.sub(sectorHash, 13) .. '0000'
            oldKey = 'ffffffffffff'
        else
            sectorKey = 'ffffffffffff'
            sectorData = '00000000000000000000000000000000'
            oldKey = string.sub(sectorHash, 1, 12)
        end
        -- Write key to sector
        -- Key data is stored in block 3 of sector (sectorNum * 4 + 3)
        print('Writing sector ' .. sectorNum .. ' key')
        execCommand('hf mf wrbl ' .. sectorNum * 4 + 3 ..' A ' .. oldKey .. ' ' .. sectorKey .. 'ff078069ffffffffffff')
        -- Write verification data to block 1 of sector (sectorNum * 4 + 1) with trailing zeroes
        print('Writing sector ' .. sectorNum .. ' data')
        execCommand('hf mf wrbl ' .. sectorNum * 4 + 1 .. ' A ' .. sectorKey .. ' ' .. sectorData)
        print('')
    end
    print('Done!')
end

tagInfo = reader.read14443a()

while tagInfo == nil or tagInfo['uid'] == nil do
    print('Can\'t select tag...')
    tagInfo = reader.read14443a()
end

if tagInfo['sak'] ~= 8 then
    print('Not a Mifare Classic tag!')
    return
end

if validateNewUid(tagInfo['uid']) then
    generateSeed()
    if utils.confirm('Write seed ' .. seed .. ' for UID ' .. tagInfo['uid'] .. ' to file and tag?') then
        writeTagSeed(tagInfo['uid'], seed)
        writeToTag(1)
    end
else
    print('Seed is already set for UID ' .. tagInfo['uid'] .. '!')
    if utils.confirm('Write to card again?') then
        -- Write same seed data again
        writeToTag(1)
    elseif utils.confirm('Reset card to defaults?') then
        writeToTag(0)
        if utils.confirm('Remove existing seed?') then
            deleteTagSeed(tagInfo['uid'])
        end
    elseif utils.confirm('Remove existing seed? (Make sure you have the keys/seed stored somewhere else if the tag is not yet reverted to its defaults!)') then
        if utils.confirm('Are you sure? READ ABOVE WARNING!') then
            deleteTagSeed(tagInfo['uid'])
        end
    end
end
