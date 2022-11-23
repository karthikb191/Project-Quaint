var canvas = document.getElementById("AllocatorCanvas");
if(!canvas.getContext)
{
    window.alert("Invalid Context");
}

var settings = document.getElementById("Settings");
var rectWidth = parseInt(settings.getAttribute("rectWidth"));
var rectHeight = parseInt(settings.getAttribute("rectHeight"));

console.log("Settings: ", rectWidth, rectHeight);

var ctx = canvas.getContext('2d');

var allocators = document.getElementsByClassName("Allocator");
var i = 0;
for(var i = 0; i< allocators.length; i++)
{
    printAllocator(allocators[i], 0, i * rectHeight + (i + 1) * 40);
}

function printAllocator(allocator, x, y)
{
    var name = allocator.getAttribute("name");
    ctx.font = "20px Arial";
    ctx.fillText(name, x, y + rectHeight + 20); 
    ctx.font = "10px Arial";

    var allocStartAddr = Number(allocator.getAttribute("addrStart")); 
    var allocEndAddr = Number(allocator.getAttribute("addrEnd")); 
    var allocAddrWidth = allocEndAddr - allocStartAddr;

    var ratio = rectWidth / allocAddrWidth;

    ctx.strokeRect(x, y, rectWidth, rectHeight);
    //console.log("ratio: ", ratio);
    //console.log("allocator pos:", x, y);
    //TODO: Add couple of fail checks
    var chunks = allocator.getElementsByClassName("Chunk");
    for(var i = 0; i < chunks.length; i++)
    {
        var chunk = chunks[i];
        var startAddr = Number(chunk.getAttribute("addrStart"));
        var endAddr = Number(chunk.getAttribute("addrEnd"));
        var chunkWidth = endAddr - startAddr;


        chunkWidth = chunkWidth * ratio;
        var chunkHeight = rectHeight;
        var xPos = x + startAddr * ratio;
        var yPos = y;
        //console.log(xPos, "   ", yPos);
        //console.log("Width and height", chunkWidth, chunkHeight);
        ctx.fillRect(xPos, yPos, chunkWidth, chunkHeight);
        
        ctx.fillText(startAddr.toString(16), xPos, yPos);
        ctx.fillText(endAddr.toString(16), x + endAddr * ratio, yPos);
    }
}
