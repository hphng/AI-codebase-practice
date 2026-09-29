// Draws the floor grid. Stops are labelled with their position in the visit order.
export default function WarehouseMap({ grid, stops, order }) {
  const labelAt = new Map();
  for (const s of stops) {
    const position = order.indexOf(s.sku);
    labelAt.set(`${s.row},${s.col}`, position === -1 ? '!' : String(position + 1));
  }

  return (
    <div className="map">
      {grid.map((row, r) => (
        <div key={r} className="map-row">
          {[...row].map((cell, c) => {
            const label = labelAt.get(`${r},${c}`);
            const kind = label ? 'stop' : cell === '#' ? 'shelf' : cell === 'S' ? 'dock' : 'floor';
            return (
              <span key={c} className={`cell ${kind}`}>
                {label || (cell === 'S' ? 'S' : '')}
              </span>
            );
          })}
        </div>
      ))}
    </div>
  );
}
