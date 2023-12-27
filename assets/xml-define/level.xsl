<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import-schema schema-location="level.xsd"/>

  <xsl:template match="/">
    <html>
      <head>
        <style>
          body {
          display: grid;
          place-items: center;
          align-content: center;
          }
          p.point {
          font-family: monospace;
          margin: 0;
          }

          h1 {
          margin: 0;
          }
        </style>
      </head>
      <body>
        <h1>
          <xsl:value-of select="level/name[text()]"/>
        </h1>
        <p>
          BG Image path: <code>
            <xsl:value-of select="level/background/@src"/>
          </code>
        </p>
        <xsl:for-each select="level/point">
          <p class="point">
            X: <xsl:value-of select="@x"/> Y: <xsl:value-of select="@y"/>
          </p>
        </xsl:for-each>
      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>