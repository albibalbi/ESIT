// File: MyDashboardPage.jsx

import React, { useState, useEffect, useRef } from 'react';
import L from 'leaflet'; // Import Leaflet library
import 'leaflet/dist/leaflet.css'; // Import Leaflet CSS

const MyDashboardPage = () => {
  const [selectedOption, setSelectedOption] = useState('');
  const [selectedData, setSelectedData] = useState(null);
  const [options, setOptions] = useState([]);
  const [jsonData, setJsonData] = useState([]);
  const [map, setMap] = useState(null); // State to hold the map instance
  const mapInitialized = useRef(false);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch('https://raw.githubusercontent.com/panciut/test/main/20240123_1.json');
        console.log(response);
        const jsonData = await response.json();

        if (Array.isArray(jsonData) && jsonData.length > 0 && jsonData[0].date) {
          const dataOptions = jsonData.map(item => item.date) || [];

          setOptions(dataOptions);
          setJsonData(jsonData);

          if (dataOptions.length > 0) {
            setSelectedOption(dataOptions[0]);
            setSelectedData(jsonData[0]);
          }

          // Create the map when data is fetched
          createMap(jsonData[0]);
        } else {
          console.error('Invalid JSON structure:', jsonData);
        }
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchData();
  }, []); // Empty dependency array ensures the effect runs only once on mount

  const handleDropdownChange = (event) => {
    const selectedDataValue = event.target.value;
    setSelectedOption(selectedDataValue);

    const selectedDataItem = jsonData.find(item => item.date === selectedDataValue);
    setSelectedData(selectedDataItem);

    // Update the map when the dropdown changes
    updateMap(selectedDataItem);
  };

  const createMap = (firstRun) => {
    if (!mapInitialized.current) {
      const mymap = L.map('map').setView([parseFloat(firstRun.positions[0].lat), parseFloat(firstRun.positions[0].lng)], 15);

      L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '© OpenStreetMap contributors'
      }).addTo(mymap);

      const coordinates = firstRun.positions.map(position => [parseFloat(position.lat), parseFloat(position.lng)]);
      const polyline = L.polyline(coordinates, { color: 'blue' }).addTo(mymap);

      mymap.fitBounds(polyline.getBounds());

      setMap(mymap); // Save the map instance in state
      mapInitialized.current = true; // Set the flag to true
    }
  };

  const updateMap = (selectedDataItem) => {
    if (map && selectedDataItem) {
      const coordinates = selectedDataItem.positions.map(position => [parseFloat(position.lat), parseFloat(position.lng)]);
      const polyline = L.polyline(coordinates, { color: 'blue' });

      // Clear previous layers and add the new polyline
      map.eachLayer(layer => {
        if (layer instanceof L.Polyline) {
          layer.remove();
        }
      });

      polyline.addTo(map);
      map.fitBounds(polyline.getBounds());
    }
  };

  return (
    <div className="dashboard-container" style={{ display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
      <div className="dropdown-container" style={{ textAlign: 'center' }}>
        <label htmlFor="dropdown">Select a date:</label>
        <select
          id="dropdown"
          value={selectedOption}
          onChange={handleDropdownChange}
          className="custom-dropdown"
          style={{ padding: '12px', fontSize: '16px', width: '300px', border: '1px solid #ddd', borderRadius: '4px' }}
        >
          {options.map((option, index) => (
            <option key={index} value={option}>
              {option}
            </option>
          ))}
        </select>
      </div>

      {/* Map container with added border */}
      <div id="map" style={{ border: '2px solid #ddd', borderRadius: '8px', margin: '20px', height: '400px', minWidth: '400px' }}></div>

      {/* Dashboard container */}
      <div className="dashboard" style={{ margin: '20px', padding: '20px', maxWidth: '600px', minWidth: '400px', textAlign: 'center' }}>
        <h2>{selectedOption}</h2>
        <p><strong>Duration:</strong> {selectedData ? `${(selectedData.duration / 60).toFixed(2)} minutes` : 'N/A'}</p>
        <p><strong>Distance:</strong> {selectedData ? `${(selectedData.distance / 1000).toFixed(2)} kilometers` : 'N/A'}</p>
        <p><strong>Average Speed:</strong> {selectedData ? `${selectedData.avg_speed} km/h` : 'N/A'}</p>
        <p><strong>Altitude Difference:</strong> {selectedData ? `${selectedData.altitude_diff} meters` : 'N/A'}</p>
      </div>
    </div>
  );
};

export default MyDashboardPage;