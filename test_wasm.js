const PCSTFastModule = require('./pcst_fast.js');
const path = require('path');
const fs = require('fs');

async function runTest() {
    const wasmBinary = fs.readFileSync(path.join(__dirname, 'pcst_fast.wasm'));
    const module = await PCSTFastModule({
        wasmBinary: wasmBinary
    });

    // Simple tree test:
    // 0 --(1)-- 1 --(1)-- 2
    // Prizes: 0: 0, 1: 10, 2: 0
    // Edges: (0, 1) cost 1, (1, 2) cost 1

    const edges = new Int32Array([0, 1, 1, 2]);
    const prizes = new Float64Array([0, 10, 0]);
    const costs = new Float64Array([1, 1]);
    const root = -1;
    const num_clusters = 1;
    const pruning = "gw";
    const verbosity = 0;

    console.log("Calling pcst_fast...");
    const result = module.pcst_fast(edges, prizes, costs, root, num_clusters, pruning, verbosity);

    console.log("Nodes in solution:", result.nodes);
    console.log("Edges in solution (indices):", result.edges);

    // Expected: node 1 should be in the solution.
    // Actually, node 1 has prize 10, costs are 1.
    // GW pruning should return node 1 and maybe others depending on how it works.

    let node1Found = false;
    for (let i = 0; i < result.nodes.length; i++) {
        if (result.nodes[i] === 1) {
            node1Found = true;
            break;
        }
    }

    if (node1Found) {
        console.log("Test Passed: Node 1 is in the solution.");
    } else {
        console.log("Test Failed: Node 1 is NOT in the solution.");
        process.exit(1);
    }
}

runTest().catch(err => {
    console.error(err);
    process.exit(1);
});
