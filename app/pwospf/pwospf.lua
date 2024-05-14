p_pwospf = Proto("pwospf", "PWOSPF")
local f = p_pwospf.fields
f.ver = ProtoField.uint8("pwospf.ver", "Version", base.DEC)
f.tp = ProtoField.uint8("pwospf.tp", "Type", base.DEC)
f.length = ProtoField.uint16("pwospf.length", "Packet length", base.DEC)
f.routerid = ProtoField.ipv4("pwospf.routerid", "Router ID")
f.areaid = ProtoField.ipv4("pwospf.areaid", "Area ID")
f.checksum = ProtoField.uint16("pwospf.checksum", "Checksum", base.HEX)
f.authtype = ProtoField.uint16("pwospf.authtype", "Autype", base.HEX)
f.authentication = ProtoField.uint64("pwospf.authentication", "Authentication", base.HEX)

f.netmask = ProtoField.ipv4("pwospf.netmask", "Network Mask")
f.helloint = ProtoField.uint16("pwospf.helloint", "HelloInt", base.DEC)
f.padding = ProtoField.uint16("pwospf.padding", "Padding", base.HEX)

f.sequnce = ProtoField.uint16("pwospf.sequnce", "Sequence", base.DEC)
f.ttl = ProtoField.uint16("pwospf.ttl", "TTL", base.DEC)
f.numadvertisements = ProtoField.uint16("pwospf.numadvertisements", "# advertisements", base.DEC)

f.lsusubnet = ProtoField.ipv4("pwospf.lsusubnet", "Subnet")
f.lsumask = ProtoField.ipv4("pwospf.lsumask", "Mask")
f.lsurouterid = ProtoField.ipv4("pwospf.lsurouterid", "Router ID")
f.lsumetric = ProtoField.uint32("pwospf.lsumetric", "Metric")

function p_pwospf.dissector(buffer, pinfo, tree)
    if buffer:len() == 0 then
        return
    end

    subtree = tree:add(p_pwospf, buffer(0))
    subtree:add(f.ver, buffer(0, 1))
    subtree:add(f.tp, buffer(1, 1))
    subtree:add(f.length, buffer(2, 2))
    subtree:add(f.routerid, buffer(4, 4))
    subtree:add(f.areaid, buffer(8, 4))
    subtree:add(f.checksum, buffer(12, 2))
    subtree:add(f.authtype, buffer(14, 2))
    subtree:add(f.authentication, buffer(16, 8))

    local tp = buffer(1, 1):uint()

    if tp == 1 then
        subtree:add(f.netmask, buffer(24, 4))
        subtree:add(f.helloint, buffer(28, 2))
        subtree:add(f.padding, buffer(30, 2))
        pinfo.cols.protocol = "PWOSPF (Hello)"
    elseif tp == 4 then
        subtree:add(f.sequnce, buffer(24, 2))
        subtree:add(f.ttl, buffer(26, 2))
        subtree:add(f.numadvertisements, buffer(28, 4))

        local numadvertisements = buffer(28, 4):uint()

        local i = 0

        while i < numadvertisements do
            local subtree2 = subtree:add('Link state advertisement ' .. i)
            subtree2:add(f.lsusubnet, buffer(32 + i * 16, 4))
            subtree2:add(f.lsumask, buffer(36 + i * 16, 4))
            subtree2:add(f.lsurouterid, buffer(40 + i * 16, 4))
            subtree2:add(f.lsumetric, buffer(44 + i * 16, 4))

            i = i + 1
        end

        pinfo.cols.protocol = "PWOSPF (LSU)"
    else
        pinfo.cols.protocol = p_pwospf.name

    end

    pinfo.cols.info = ""

end

function p_pwospf.init()
end

local ip_dissector_table = DissectorTable.get("ip.proto")
ip_dissector_table:add(89, p_pwospf)
